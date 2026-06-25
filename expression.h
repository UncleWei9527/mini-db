//
// Created by wjh on 2026/6/21.
//

#pragma  once
#include"value.h"
#include"tuple.h"
#include"catalog.h"

namespace minidb {
    class AbstractExpression {
    public:
        virtual ~AbstractExpression() = default;
        virtual bool Valid()const=0;
        virtual std::string ToString()const =0;
        virtual Value Evaluate(const Tuple *tuple, const Schema *schema) const =0;
    };

    class ConstantValueExpression : public AbstractExpression {
    public:
        ConstantValueExpression(Value val);
        bool Valid()const;
        std::string ToString()const;
        virtual Value Evaluate(const Tuple *tuple, const Schema *schema) const;

    private:
        Value val_;
    };

    class ColumnValueExpression : public AbstractExpression {
    public:
        ColumnValueExpression( const std::string &col_name);
        bool Valid()const;
        std::string ToString()const;
        virtual Value Evaluate(const Tuple *tuple, const Schema *schema) const;

    private:
        std::string col_name_;
    };
    enum class CompareOp {
        Lt,
        Le,
        Gt,
        Ge,
        Ne,
        Eq
    };
    class ComparisonExpression : public AbstractExpression {
    public:
        ComparisonExpression(AbstractExpression*left,AbstractExpression*right,CompareOp op);
        virtual Value Evaluate(const Tuple *tuple, const Schema *schema) const;
        bool Valid()const;
        std::string ToString()const;
    private:
        AbstractExpression*left_,*right_;
        CompareOp op_;
    };
}
