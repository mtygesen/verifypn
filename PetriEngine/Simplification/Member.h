#ifndef MEMBER_H
#define MEMBER_H
#include <algorithm>
#include <functional>
//#include "../PQL/PQL.h"

namespace PetriEngine {
    namespace Simplification {
        enum Trivial { False=0, True=1, Indeterminate=2 };
        enum MemberType { Constant, Input, Output, Regular };
        class Member {
            std::vector<int> _variables;
            int _constant;
            bool _canAnalyze;

        public:

            Member(std::vector<int> vec, int constant) : _variables(vec), _constant(constant) {
                _canAnalyze = true;
            }
            Member(int constant) : _constant(constant) {
                _canAnalyze = true;
            }
            Member(){}

            virtual ~Member(){}
            
            int constant() const   { return _constant; };
            bool canAnalyze() const { return _canAnalyze; };
            size_t size() const     { return _variables.size(); }
            std::vector<int>& variables() { return _variables; }
            
            Member operator+(const Member& m) const { 
                Member res;
                res._constant = _constant + m._constant;
                res._variables = addVariables(*this, m);
                res._canAnalyze = _canAnalyze&&m._canAnalyze;
                return res;
            }
            Member operator-(const Member& m) const { 
                Member res;
                res._constant = _constant - m._constant;
                res._variables = subtractVariables(*this, m);
                res._canAnalyze = _canAnalyze&&m._canAnalyze;
                return res;
            }
            Member operator*(const Member& m) const { 
                Member res;
                if(!isConstant()&&!m.isConstant()){
                    res._canAnalyze = false;
                } else{
                    res._constant = _constant * m._constant;            
                    res._canAnalyze = _canAnalyze&&m._canAnalyze;
                    res._variables = multiply(*this, m);
                }
                return res;
            }
            Member operator-() const { 
                Member res;
                Member negative(-1);
                res = *this * negative;
                return res;
            }

            bool isConstant() const {
                for(const int& v : _variables){
                    if(v != 0) return false;
                }
                return true;
            }
            
            MemberType getType() const {
                bool isConstant=true;
                bool isInput=true;
                bool isOutput=true;
                for(const int& v : _variables){
                    if(v < 0){
                        isConstant=false;
                        isOutput=false;
                    } else if(v > 0){
                        isConstant=false;
                        isInput=false;
                    }
                }
                if(isConstant) return MemberType::Constant;
                else if(isInput) return MemberType::Input;
                else if(isOutput) return MemberType::Output;
                else return MemberType::Regular;
            }
            
            bool operator==(const Member& m) const {
                return (*this-m).isConstant();
            }
            bool operator!=(const Member& m) const {
                return !((*this-m).isConstant());
            }
            Trivial operator<(const Member& m) const {
                return trivialLessThan(*this, m, std::less<int>());
            }
            Trivial operator<=(const Member& m) const {
                return trivialLessThan(*this, m, std::less_equal<int>());
            }
            Trivial operator>(const Member& m) const {
                return trivialLessThan(m, *this, std::less<int>());
            }
            Trivial operator>=(const Member& m) const {
                return trivialLessThan(m, *this, std::less_equal<int>());
            }
            
        private:
            std::vector<int> addVariables(const Member& m1, const Member& m2) const {
                uint32_t size = std::max(m1._variables.size(), m2._variables.size());
                std::vector<int> res(size);
                
                for (uint32_t i = 0; i < size; i++) {
                    if (i + 1 > m1._variables.size()) {
                        res[i] = m2._variables[i];
                    } else if (i + 1 > m2._variables.size()) {
                        res[i] = m1._variables[i];
                    } else {
                        res[i] = m1._variables[i] + m2._variables[i];
                    }
                }
                
                return res;
            }

            std::vector<int> subtractVariables(const Member& m1, const Member& m2) const {
                uint32_t size = std::max(m1._variables.size(), m2._variables.size());
                std::vector<int> res(size);
                
                for (uint32_t i = 0; i < size; i++) {
                    if (i + 1 > m1._variables.size()) {
                        res[i] =  0 - m2._variables[i];
                    } else if (i + 1 > m2._variables.size()) {
                        res[i] = m1._variables[i];
                    } else {
                        res[i] = m1._variables[i] - m2._variables[i];
                    }
                }
                
                return res;
            }

            std::vector<int> multiply(const Member& m1, const Member& m2) const {
                std::vector<int> res;
                if (m1.isConstant() != m2.isConstant()){
                    if (!m1.isConstant()){
                        for(auto& v : m1._variables){
                            res.push_back(v * m2._constant);
                        }
                    } else if (!m2.isConstant()){
                        for (auto& v : m2._variables){
                            res.push_back(v * m1._constant);
                        }
                    }
                }
                return res;
            }
            
            Trivial trivialLessThan(const Member& m1, const Member& m2, std::function<bool (int, int)> compare) const {
                MemberType type1 = m1.getType();
                MemberType type2 = m2.getType();
                
                // self comparison
                if (m1 == m2)
                    return compare(m1._constant, m2._constant) ? Trivial::True : Trivial::False;
                    
                // constant < constant/input/output
                if (type1 == MemberType::Constant){
                    if(type2 == MemberType::Constant){
                        return compare(m1._constant, m2._constant) ? Trivial::True : Trivial::False;
                    }
                    else if(type2 == MemberType::Input && !compare(m1._constant, m2._constant)){
                        return Trivial::False;
                    } 
                    else if(type2 == MemberType::Output && compare(m1._constant, m2._constant)){
                        return Trivial::True;
                    } 
                } 
                // input < output/constant
                else if (type1 == MemberType::Input 
                        && (type2 == MemberType::Constant || type2 == MemberType::Output) 
                        && compare(m1._constant, m2._constant)) {
                    return Trivial::True;
                } 
                // output < input/constant
                else if (type1 == MemberType::Output 
                        && (type2 == MemberType::Constant || type2 == MemberType::Input) 
                        && !compare(m1._constant, m2._constant)) {
                    return Trivial::False;
                } 
                return Trivial::Indeterminate;
            }

        };
    }
}

#endif /* MEMBER_H */
