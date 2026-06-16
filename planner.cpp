//
// Created by wjh on 2026/6/15.
//
#include"planner.h"

std::unique_ptr<minidb::AbstractExecutor> minidb::Planner::Plan(SQLStatement *ast)  {
    if (ast->GetType() == StatementType::SELECT) {
        return PlanSelect(static_cast<SelectStatement*>(ast));
    } else if (ast->GetType() == StatementType::INSERT) {
        return PlanInsert(static_cast<InsertStatement*>(ast));
    }
    throw std::runtime_error("Planner Error: Unsupported AST node.");
}

std::unique_ptr<minidb::AbstractExecutor> minidb::Planner::PlanSelect(SelectStatement *stmt) {
    TableHeap *table = catalog_->GetTableHeap(stmt->table_name_);
    const std::vector<TypeId>&type_ids=catalog_->GetSchema(stmt->table_name_).GetSchemaType();
    return std::make_unique<SeqScanExecutor>(table, type_ids);
}

std::unique_ptr<minidb::AbstractExecutor> minidb::Planner::PlanInsert(InsertStatement *stmt) {
    TableHeap *target_table = catalog_->GetTableHeap(stmt->table_name_);
    std::unique_ptr<AbstractExecutor> child;
    if (stmt->has_values_) {
        child=std::make_unique<ValuesExecutor>(stmt->raw_values_);
    }
    else
     child = Plan(stmt->select_query_.get());
    return std::make_unique<InsertExecutor>(child.release(), target_table);
}