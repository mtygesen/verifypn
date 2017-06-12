#include <assert.h>
#include "../../lpsolve/lp_lib.h"
#include "LinearProgram.h"
#include "LPCache.h"

namespace PetriEngine {
    namespace Simplification {

        LinearProgram::~LinearProgram() {
        }
        
        LinearProgram::LinearProgram(Vector* vec, int constant, op_t op, LPCache* factory){
            // TODO fix memory-management here!
            equation_t c;
            switch(op)
            {
                case Simplification::OP_LT:
                    c.upper = constant - 1;
                    break;
                case Simplification::OP_GT:
                    c.lower = constant + 1;
                    break;
                case Simplification::OP_LE:
                    c.upper = constant;
                    break;
                case Simplification::OP_GE:
                    c.lower = constant;
                    break;
                case Simplification::OP_EQ:
                    c.lower = constant;
                    c.upper = constant;
                    break;
                default:
                    // We ignore this operator for now by not adding any equation.
                    // This is untested, however.
                    assert(false);
            }
            c.row = vec;
            vec->inc();
            _equations.push_back(c);
        }
        
        bool LinearProgram::isImpossible(const PetriEngine::PetriNet* net, const PetriEngine::MarkVal* m0, uint32_t timeout, bool use_ilp) {
           
            if(_result != result_t::UKNOWN) 
            {
                if(!use_ilp || _result == result_t::IMPOSSIBLE)
                    return _result == result_t::IMPOSSIBLE;
            }

            if(_equations.size() == 0){
                return false;
            }

            
            const uint32_t nCol = net->numberOfTransitions();
            lprec* lp;
            int nRow = net->numberOfPlaces() + (_equations.size() * 2);
            
            lp = make_lp(nRow, nCol);
            assert(lp);
            if (!lp) return false;
            set_verbose(lp, IMPORTANT);

            set_add_rowmode(lp, TRUE);
            
            std::vector<REAL> row = std::vector<REAL>(nCol + 1);
            REAL constant = 0;
            int comparator = GE;
            
            int rowno = 1;
            
            // restrict all places to contain 0+ tokens
            for (size_t p = 0; p < net->numberOfPlaces(); p++) {
                memset(row.data(), 0, sizeof (REAL) * (nCol + 1));
                for (size_t t = 0; t < nCol; t++) {
                    row[1 + t] = net->outArc(t, p) - net->inArc(p, t);
                }
                set_row(lp, rowno, row.data());
                set_constr_type(lp, rowno, GE);
                set_rh(lp, rowno++, (0 - (int)m0[p]));
            }
            for(const auto& eq : _equations){
                eq.row->write(row);               

                for(size_t mode : {0, 1})
                {
                    if(std::isinf(eq[mode])) continue;

                    if(mode == 1 && eq.upper == eq.lower) continue;
                    
                    if(eq.upper == eq.lower) comparator = EQ;
                    else if(mode == 0) comparator = GE;
                    else /* mode == 1*/comparator = LE;

                    constant = eq[mode];

                    set_row(lp, rowno, row.data());
                    set_constr_type(lp, rowno, comparator);
                    set_rh(lp, rowno++, constant);
                }
            }

            set_add_rowmode(lp, FALSE);
            
            // Create objective
            memset(row.data(), 0, sizeof (REAL) * net->numberOfTransitions() + 1);
            for (size_t t = 0; t < net->numberOfTransitions(); t++)
                row[1 + t] = 1; // The sum the components in the firing vector

            // Set objective
            set_obj_fn(lp, row.data());

            // Minimize the objective
            set_minim(lp);

            for (size_t i = 0; i < nCol; i++){
                set_int(lp, 1 + i, use_ilp ? TRUE : false);
            }
            
            set_timeout(lp, timeout);
            set_break_at_first(lp, TRUE);
            set_presolve(lp, PRESOLVE_ROWS | PRESOLVE_COLS | PRESOLVE_LINDEP, get_presolveloops(lp));
//            write_LP(lp, stdout);
            int result = solve(lp);
            delete_lp(lp);

            if (result == TIMEOUT) std::cout << "note: lpsolve timeout" << std::endl;
            // Return true, if it was infeasible
            if(result == INFEASIBLE)
            {
                _result = result_t::IMPOSSIBLE;
            }
            else
            {
                _result = result_t::POSSIBLE;
            }
           return _result == result_t::IMPOSSIBLE;
        }
        
        
        void LinearProgram::make_union(const LinearProgram& other)
        {
            if(_result == IMPOSSIBLE || other._result == IMPOSSIBLE)
            {
                _result = IMPOSSIBLE;
                _equations.clear();
                assert(false);
                return;
            }

            auto it1 = _equations.begin();
            auto it2 = other._equations.begin();

            while(it1 != _equations.end() && it2 != other._equations.end())
            {
                if(it1->row < it2->row)
                {
                    ++it1;
                }
                else if(it2->row < it1->row)
                {
                    it1 = _equations.insert(it1, *it2);
                    ++it2;
                    ++it1;
                }
                else
                {
                    equation_t& n = *it1;
                    n.lower = std::max(n.lower, it2->lower);
                    n.upper = std::min(n.upper, it2->upper);
                    /*if(n.upper < n.lower)
                    {
                        _result = result_t::IMPOSSIBLE;
                        _equations.clear();
                        return;
                    }*/
                    ++it1;
                    ++it2;
                }                    
            }

            if(it2 != other._equations.end()) 
                _equations.insert(_equations.end(), it2, other._equations.end());
        }
    }
}

