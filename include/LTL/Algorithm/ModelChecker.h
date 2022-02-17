/* Copyright (C) 2020  Nikolaj J. Ulrik <nikolaj@njulrik.dk>,
 *                     Simon M. Virenfeldt <simon@simwir.dk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VERIFYPN_MODELCHECKER_H
#define VERIFYPN_MODELCHECKER_H

#include "PetriEngine/PQL/PQL.h"
#include "LTL/SuccessorGeneration/ProductSuccessorGenerator.h"
#include "LTL/SuccessorGeneration/ReachStubProductSuccessorGenerator.h"
#include "LTL/SuccessorGeneration/ResumingSuccessorGenerator.h"
#include "LTL/SuccessorGeneration/SpoolingSuccessorGenerator.h"
#include "LTL/Structures/BitProductStateSet.h"
#include "LTL/SuccessorGeneration/ReachStubProductSuccessorGenerator.h"
#include "LTL/Structures/ProductStateFactory.h"
#include "PetriEngine/options.h"
#include "PetriEngine/Reducer.h"

#include <iomanip>
#include <algorithm>

namespace LTL {

    class ModelChecker {
    public:

        ModelChecker(const PetriEngine::PetriNet& net,
                const PetriEngine::PQL::Condition_ptr &condition,
                const Structures::BuchiAutomaton &buchi, bool built_trace)
        : _net(net), _formula(condition),
          _factory(net, buchi), _buchi(buchi), _built_trace(built_trace) {
        }

        void set_utilize_weak(bool b) {
            _shortcircuitweak = b;
        }

        virtual bool check() = 0;

        virtual ~ModelChecker() = default;

        [[nodiscard]] bool is_weak() const {
            return _is_weak;
        }

        size_t get_explored() {
            return _explored;
        }

        virtual void print_stats(std::ostream&) = 0;

        size_t loop_index() const {
            return _loop;
        }

        const std::vector<size_t>& trace() const {
            return _trace;
        }


    protected:
        size_t _explored = 0;
        size_t _expanded = 0;

        virtual void print_stats(std::ostream &os, const LTL::Structures::ProductStateSetInterface &stateSet) {
            std::cout << "STATS:\n"
                    << "\tdiscovered states: " << stateSet.discovered() << std::endl
                    << "\texplored states:   " << _explored << std::endl
                    << "\texpanded states:   " << _expanded << std::endl
                    << "\tmax tokens:        " << stateSet.max_tokens() << std::endl;
        }


        const PetriEngine::PetriNet& _net;
        PetriEngine::PQL::Condition_ptr _formula;
        Structures::ProductStateFactory _factory;
        const Structures::BuchiAutomaton& _buchi;
        size_t _discovered = 0;
        bool _shortcircuitweak;
        bool _weakskip = false;
        bool _is_weak = false;
        bool _built_trace = false;

        size_t _loop = std::numeric_limits<size_t>::max();
        std::vector<size_t> _trace;
    };
}

#endif //VERIFYPN_MODELCHECKER_H
