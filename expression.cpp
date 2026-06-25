//
// Created by wjh on 2026/6/21.
//

#include "expression.h"
#include <vector>
#include <algorithm>
#include<format>
#include<map>
minidb::ConstantValueExpression::ConstantValueExpression(Value val)
    :val_(val)
{
}

bool minidb::ConstantValueExpression::Valid() const {
    return val_.GetTypeId()!=TypeId::INVALID;
}

std::string minidb::ConstantValueExpression::ToString() const {
    return val_.ToString();
}

minidb::Value minidb::ConstantValueExpression::Evaluate(const Tuple *tuple, const Schema *schema) const {
    return val_;
}

minidb::ColumnValueExpression::ColumnValueExpression( const std::string &col_name)
    :col_name_(col_name)
{

}

minidb::ComparisonExpression::ComparisonExpression(AbstractExpression *left, AbstractExpression *right, CompareOp op)
    :left_(left),right_(right),op_(op)
{

}

minidb::Value minidb::ComparisonExpression::Evaluate(const Tuple *tuple, const Schema *schema) const {
    Value left_result=left_->Evaluate(tuple,schema);
    Value right_result=right_->Evaluate(tuple,schema);
    switch (op_) {
        case CompareOp::Eq:
            return Value(left_result.CompareEQ(right_result));
        case CompareOp::Ne:
            return Value(left_result.CompareNEQ(right_result));
        case CompareOp::Lt:
            return Value(left_result.CompareLT(right_result));
        case CompareOp::Gt:
            return Value(left_result.CompareGT(right_result));
        case CompareOp::Ge:
            return Value(left_result.CompareGE(right_result));
        case CompareOp::Le:
            return Value(left_result.CompareLE(right_result));
    }
    throw std::runtime_error("unreachable ");
}

bool minidb::ComparisonExpression::Valid() const {
    return left_->Valid()&&right_->Valid();
}

std::string minidb::ComparisonExpression::ToString() const {
    static std::map<CompareOp,std::string>op_strs={
        {CompareOp::Lt, "<"},
        {CompareOp::Le, "<="},
        {CompareOp::Gt, ">"},
        {CompareOp::Ge, ">="},
        {CompareOp::Ne, "!="},
        {CompareOp::Eq,"=="},
    };
    return std::format("{}{}{}",left_->ToString(),op_strs[op_],right_->ToString());
}

bool minidb::ColumnValueExpression::Valid() const {
   // return col_idx_!=-1;
    //todo:做个检查
    return true;
}

std::string minidb::ColumnValueExpression::ToString() const {
    return col_name_;
}

minidb::Value minidb::ColumnValueExpression::Evaluate(const Tuple *tuple, const Schema *schema) const {
    const auto &columns=schema->GetColumns();
    auto it=std::find_if(columns.begin(),columns.end(),
    [this](const Column&c) {
        return c.column_name_==col_name_;
    }
    );
    uint32_t col_idx=-1;
    if (it!=schema->GetColumns().end()) {
        col_idx=std::distance(columns.begin(),it);
    }
    else throw std::logic_error(std::format("unknown column name {}",col_name_));
    return tuple->GetValues()[col_idx];
}
