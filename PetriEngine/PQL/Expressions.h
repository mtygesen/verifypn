/* PeTe - Petri Engine exTremE
 * Copyright (C) 2011  Jonas Finnemann Jensen <jopsen@gmail.com>,
 *                     Thomas Søndersø Nielsen <primogens@gmail.com>,
 *                     Lars Kærlund Østergaard <larsko@gmail.com>,
 *                     Peter Gjøl Jensen <root@petergjoel.dk>
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
#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include "PQL.h"
#include "Contexts.h"

namespace PetriEngine {

    namespace Structures {
        class StateConstraints;
    }

    namespace PQL {


        /******************** EXPRESSIONS ********************/

        /** Base class for all binary expressions */
        class BinaryExpr : public Expr {
        public:

            BinaryExpr(const Expr_ptr expr1, const Expr_ptr expr2) {
                _expr1 = expr1;
                _expr2 = expr2;
            }
            void analyze(AnalysisContext& context);
            bool pfree() const;
            int evaluate(const EvaluationContext& context) const;
            int evalAndSet(const EvaluationContext& context);
            //llvm::Value* codegen(CodeGenerationContext& context) const;
            std::string toString() const;
        protected:
            virtual int apply(int v1, int v2) const = 0;
            /** LLVM binary operator (llvm::Instruction::BinaryOps) */
            //virtual int binaryOp() const = 0;
            virtual std::string op() const = 0;
            Expr_ptr _expr1;
            Expr_ptr _expr2;
        };

        /** Binary plus expression */
        class PlusExpr : public BinaryExpr {
        public:

            using BinaryExpr::BinaryExpr;
            Expr::Types type() const;
            void incr(ReducingSuccessorGenerator& generator) const;
            void decr(ReducingSuccessorGenerator& generator) const;
        private:
            int apply(int v1, int v2) const;
            //int binaryOp() const;
            std::string op() const;
        };

        /** Binary minus expression */
        class SubtractExpr : public BinaryExpr {
        public:

            using BinaryExpr::BinaryExpr;
            Expr::Types type() const;
            void incr(ReducingSuccessorGenerator& generator) const;
            void decr(ReducingSuccessorGenerator& generator) const;
        private:
            int apply(int v1, int v2) const;
            //int binaryOp() const;
            std::string op() const;
        };

        /** Binary multiplication expression **/
        class MultiplyExpr : public BinaryExpr {
        public:

            using BinaryExpr::BinaryExpr;
            Expr::Types type() const;
            void incr(ReducingSuccessorGenerator& generator) const;
            void decr(ReducingSuccessorGenerator& generator) const;
        private:
            int apply(int v1, int v2) const;
            //int binaryOp() const;
            std::string op() const;
        };

        /** Unary minus expression*/
        class MinusExpr : public Expr {
        public:

            MinusExpr(const Expr_ptr expr) {
                _expr = expr;
            }
            void analyze(AnalysisContext& context);
            bool pfree() const;
            int evaluate(const EvaluationContext& context) const;
            int evalAndSet(const EvaluationContext& context);
            //llvm::Value* codegen(CodeGenerationContext& context) const;
            std::string toString() const;
            Expr::Types type() const;
            void incr(ReducingSuccessorGenerator& generator) const;
            void decr(ReducingSuccessorGenerator& generator) const;
        private:
            Expr_ptr _expr;
        };

        /** Literal integer value expression */
        class LiteralExpr : public Expr {
        public:

            LiteralExpr(int value) : _value(value) {
            }
            void analyze(AnalysisContext& context);
            bool pfree() const;
            int evaluate(const EvaluationContext& context) const;
            int evalAndSet(const EvaluationContext& context);
            //llvm::Value* codegen(CodeGenerationContext& context) const;
            std::string toString() const;
            Expr::Types type() const;
            void incr(ReducingSuccessorGenerator& generator) const;
            void decr(ReducingSuccessorGenerator& generator) const;

            int value() const {
                return _value;
            };
        private:
            int _value;
        };

        /** Identifier expression */
        class IdentifierExpr : public Expr {
        public:

            IdentifierExpr(const std::string& name) : _name(name) {
                _offsetInMarking = -1;
            }
            void analyze(AnalysisContext& context);
            bool pfree() const;
            int evaluate(const EvaluationContext& context) const;
            int evalAndSet(const EvaluationContext& context);
            //llvm::Value* codegen(CodeGenerationContext& context) const;
            std::string toString() const;
            Expr::Types type() const;
            void incr(ReducingSuccessorGenerator& generator) const;
            void decr(ReducingSuccessorGenerator& generator) const;

            /** Offset in marking or valuation */
            int offset() const {
                return _offsetInMarking;
            }
        private:
            /** Offset in marking, -1 if undefined, should be resolved during analysis */
            int _offsetInMarking;
            /** Identifier text */
            std::string _name;
        };

        /******************** CONDITIONS ********************/

        /* Logical conditon */
        class LogicalCondition : public Condition {
        public:

            LogicalCondition(const Condition_ptr cond1, const Condition_ptr cond2) {
                _cond1 = cond1;
                _cond2 = cond2;
            }
            void analyze(AnalysisContext& context);
            bool evaluate(const EvaluationContext& context) const;
            bool evalAndSet(const EvaluationContext& context);
            void findConstraints(ConstraintAnalysisContext& context) const;
            //llvm::Value* codegen(CodeGenerationContext& context) const;
            uint32_t distance(DistanceContext& context) const;
            std::string toString() const;
            std::string toTAPAALQuery(TAPAALConditionExportContext& context) const;
        private:
            virtual bool apply(bool b1, bool b2) const = 0;
            /** LLVM binary operator (llvm::Instruction::BinaryOps) */
            //virtual int logicalOp() const = 0;
            virtual uint32_t delta(uint32_t d1, uint32_t d2, const DistanceContext& context) const = 0;
            virtual std::string op() const = 0;
            virtual void mergeConstraints(ConstraintAnalysisContext::ConstraintSet& result, ConstraintAnalysisContext::ConstraintSet& other, bool negated) const = 0;
        protected:
            Condition_ptr _cond1;
            Condition_ptr _cond2;
        };

        /* Conjunctive and condition */
        class AndCondition : public LogicalCondition {
        public:

            using LogicalCondition::LogicalCondition;
            Condition_ptr resolveNegation(bool negated) const;
            Condition_ptr resolveOrphans(std::vector<std::pair<std::string, uint32_t>> orphans) const;
            Condition_ptr seekAndDestroy(ConstraintAnalysisContext& context) const;
            void findInteresting(ReducingSuccessorGenerator& generator, bool negated) const;
        private:
            bool apply(bool b1, bool b2) const;
            //int logicalOp() const;
            uint32_t delta(uint32_t d1, uint32_t d2, const DistanceContext& context) const;
            void mergeConstraints(ConstraintAnalysisContext::ConstraintSet& result, ConstraintAnalysisContext::ConstraintSet& other, bool negated) const;
            std::string op() const;
        };

        /* Disjunctive or conditon */
        class OrCondition : public LogicalCondition {
        public:

            using LogicalCondition::LogicalCondition;
            Condition_ptr resolveNegation(bool negated) const;
            Condition_ptr resolveOrphans(std::vector<std::pair<std::string, uint32_t>> orphans) const;
            Condition_ptr seekAndDestroy(ConstraintAnalysisContext& context) const;
            void findInteresting(ReducingSuccessorGenerator& generator, bool negated) const;   
        private:
            bool apply(bool b1, bool b2) const;
            //int logicalOp() const;
            uint32_t delta(uint32_t d1, uint32_t d2, const DistanceContext& context) const;
            void mergeConstraints(ConstraintAnalysisContext::ConstraintSet& result, ConstraintAnalysisContext::ConstraintSet& other, bool negated) const;
            std::string op() const;
        };

        /* Comparison conditon */
        class CompareCondition : public Condition {
        public:

            CompareCondition(const Expr_ptr expr1, const Expr_ptr expr2) {
                _expr1 = expr1;
                _expr2 = expr2;
            }
            void analyze(AnalysisContext& context);
            bool evaluate(const EvaluationContext& context) const;
            bool evalAndSet(const EvaluationContext& context);
            void findConstraints(ConstraintAnalysisContext& context) const;
            //llvm::Value* codegen(CodeGenerationContext& context) const;
            uint32_t distance(DistanceContext& context) const;
            std::string toString() const;
            std::string toTAPAALQuery(TAPAALConditionExportContext& context) const;
        private:
            virtual bool apply(int v1, int v2) const = 0;
            /** LLVM Comparison predicate (llvm::ICmpInst::Predicate) */
            //virtual int compareOp() const = 0;
            virtual uint32_t delta(int v1, int v2, bool negated) const = 0;
            virtual std::string op() const = 0;
            /** Operator when exported to TAPAAL */
            virtual std::string opTAPAAL() const = 0;
            /** Swapped operator when exported to TAPAAL, e.g. operator when operands are swapped */
            virtual std::string sopTAPAAL() const = 0;
            virtual void addConstraints(ConstraintAnalysisContext& context, const Expr_ptr& id, int value) const = 0;
            virtual void addConstraints(ConstraintAnalysisContext& context, int value, const Expr_ptr& id) const = 0;
        protected:
            Expr_ptr _expr1;
            Expr_ptr _expr2;
        };

        /* Equality conditon */
        class EqualCondition : public CompareCondition {
        public:

            using CompareCondition::CompareCondition;
            Condition_ptr resolveNegation(bool negated) const;
            Condition_ptr resolveOrphans(std::vector<std::pair<std::string, uint32_t>> orphans) const;
            Condition_ptr seekAndDestroy(ConstraintAnalysisContext& context) const;
            void findInteresting(ReducingSuccessorGenerator& generator, bool negated) const;
        private:
            bool apply(int v1, int v2) const;
            //int compareOp() const;
            uint32_t delta(int v1, int v2, bool negated) const;
            void addConstraints(ConstraintAnalysisContext& context, const Expr_ptr& id, int value) const;
            void addConstraints(ConstraintAnalysisContext& context, int value, const Expr_ptr& id) const;
            std::string op() const;
            std::string opTAPAAL() const;
            std::string sopTAPAAL() const;
        };

        /* None equality conditon */
        class NotEqualCondition : public CompareCondition {
        public:

            using CompareCondition::CompareCondition;
            std::string toTAPAALQuery(TAPAALConditionExportContext& context) const;
            Condition_ptr resolveNegation(bool negated) const;
            Condition_ptr resolveOrphans(std::vector<std::pair<std::string, uint32_t>> orphans) const;
            Condition_ptr seekAndDestroy(ConstraintAnalysisContext& context) const;
            void findInteresting(ReducingSuccessorGenerator& generator, bool negated) const;
        private:
            bool apply(int v1, int v2) const;
            //int compareOp() const;
            uint32_t delta(int v1, int v2, bool negated) const;
            void addConstraints(ConstraintAnalysisContext& context, const Expr_ptr& id, int value) const;
            void addConstraints(ConstraintAnalysisContext& context, int value, const Expr_ptr& id) const;
            std::string op() const;
            std::string opTAPAAL() const;
            std::string sopTAPAAL() const;
        };

        /* Less-than conditon */
        class LessThanCondition : public CompareCondition {
        public:

            using CompareCondition::CompareCondition;
            Condition_ptr resolveNegation(bool negated) const;
            Condition_ptr resolveOrphans(std::vector<std::pair<std::string, uint32_t>> orphans) const;
            Condition_ptr seekAndDestroy(ConstraintAnalysisContext& context) const;
            void findInteresting(ReducingSuccessorGenerator& generator, bool negated) const;
        private:
            bool apply(int v1, int v2) const;
            //int compareOp() const;
            uint32_t delta(int v1, int v2, bool negated) const;
            void addConstraints(ConstraintAnalysisContext& context, const Expr_ptr& id, int value) const;
            void addConstraints(ConstraintAnalysisContext& context, int value, const Expr_ptr& id) const;
            std::string op() const;
            std::string opTAPAAL() const;
            std::string sopTAPAAL() const;
        };

        /* Less-than-or-equal conditon */
        class LessThanOrEqualCondition : public CompareCondition {
        public:

            using CompareCondition::CompareCondition;
            Condition_ptr resolveNegation(bool negated) const;
            Condition_ptr resolveOrphans(std::vector<std::pair<std::string, uint32_t>> orphans) const;
            Condition_ptr seekAndDestroy(ConstraintAnalysisContext& context) const;
            void findInteresting(ReducingSuccessorGenerator& generator, bool negated) const;
        private:
            bool apply(int v1, int v2) const;
            //int compareOp() const;
            uint32_t delta(int v1, int v2, bool negated) const;
            void addConstraints(ConstraintAnalysisContext& context, const Expr_ptr& id, int value) const;
            void addConstraints(ConstraintAnalysisContext& context, int value, const Expr_ptr& id) const;
            std::string op() const;
            std::string opTAPAAL() const;
            std::string sopTAPAAL() const;
        };

        /* Greater-than conditon */
        class GreaterThanCondition : public CompareCondition {
        public:

            using CompareCondition::CompareCondition;
            Condition_ptr resolveNegation(bool negated) const;
            Condition_ptr resolveOrphans(std::vector<std::pair<std::string, uint32_t>> orphans) const;
            Condition_ptr seekAndDestroy(ConstraintAnalysisContext& context) const;
            void findInteresting(ReducingSuccessorGenerator& generator, bool negated) const;
        private:
            bool apply(int v1, int v2) const;
            //int compareOp() const;
            uint32_t delta(int v1, int v2, bool negated) const;
            void addConstraints(ConstraintAnalysisContext& context, const Expr_ptr& id, int value) const;
            void addConstraints(ConstraintAnalysisContext& context, int value, const Expr_ptr& id) const;
            std::string op() const;
            std::string opTAPAAL() const;
            std::string sopTAPAAL() const;
        };

        /* Greater-than-or-equal conditon */
        class GreaterThanOrEqualCondition : public CompareCondition {
        public:
            using CompareCondition::CompareCondition;
            Condition_ptr resolveNegation(bool negated) const;
            Condition_ptr resolveOrphans(std::vector<std::pair<std::string, uint32_t>> orphans) const;
            Condition_ptr seekAndDestroy(ConstraintAnalysisContext& context) const;
            void findInteresting(ReducingSuccessorGenerator& generator, bool negated) const;
        private:
            bool apply(int v1, int v2) const;
            //int compareOp() const;
            uint32_t delta(int v1, int v2, bool negated) const;
            void addConstraints(ConstraintAnalysisContext& context, const Expr_ptr& id, int value) const;
            void addConstraints(ConstraintAnalysisContext& context, int value, const Expr_ptr& id) const;
            std::string op() const;
            std::string opTAPAAL() const;
            std::string sopTAPAAL() const;
        };

        /* Not condition */
        class NotCondition : public Condition {
        public:

            NotCondition(const Condition_ptr cond) {
                _cond = cond;
            }
            void analyze(AnalysisContext& context);
            bool evaluate(const EvaluationContext& context) const;
            bool evalAndSet(const EvaluationContext& context);
            void findConstraints(ConstraintAnalysisContext& context) const;
            //llvm::Value* codegen(CodeGenerationContext& context) const;
            uint32_t distance(DistanceContext& context) const;
            std::string toString() const;
            std::string toTAPAALQuery(TAPAALConditionExportContext& context) const;
            Condition_ptr resolveNegation(bool negated) const;
            Condition_ptr resolveOrphans(std::vector<std::pair<std::string, uint32_t>> orphans) const;
            Condition_ptr seekAndDestroy(ConstraintAnalysisContext& context) const;
            void findInteresting(ReducingSuccessorGenerator& generator, bool negated) const;
        private:
            Condition_ptr _cond;
        };

        /* Bool condition */
        class BooleanCondition : public Condition {
        public:

            BooleanCondition(bool value) : _value(value) {
            }
            void analyze(AnalysisContext& context);
            bool evaluate(const EvaluationContext& context) const;
            bool evalAndSet(const EvaluationContext& context);
            void findConstraints(ConstraintAnalysisContext& context) const;
            uint32_t distance(DistanceContext& context) const;
            std::string toString() const;
            std::string toTAPAALQuery(TAPAALConditionExportContext& context) const;
            static Condition_ptr TRUE;
            static Condition_ptr FALSE;
            Condition_ptr resolveNegation(bool negated) const;
            Condition_ptr resolveOrphans(std::vector<std::pair<std::string, uint32_t>> orphans) const;
            Condition_ptr seekAndDestroy(ConstraintAnalysisContext& context) const;
            void findInteresting(ReducingSuccessorGenerator& generator, bool negated) const;
        private:
            const bool _value;
        };

        /* Deadlock condition */
        class DeadlockCondition : public Condition {
        public:

            DeadlockCondition() {
            }
            void analyze(AnalysisContext& context);
            bool evaluate(const EvaluationContext& context) const;
            bool evalAndSet(const EvaluationContext& context);
            void findConstraints(ConstraintAnalysisContext& context) const;
            uint32_t distance(DistanceContext& context) const;
            std::string toString() const;
            std::string toTAPAALQuery(TAPAALConditionExportContext& context) const;
            Condition_ptr resolveNegation(bool negated) const;
            Condition_ptr resolveOrphans(std::vector<std::pair<std::string, uint32_t>> orphans) const;
            Condition_ptr seekAndDestroy(ConstraintAnalysisContext& context) const;
            void findInteresting(ReducingSuccessorGenerator& generator, bool negated) const;
        };

    }
}



#endif // EXPRESSIONS_H
