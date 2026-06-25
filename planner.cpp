//
// Created by wjh on 2026/6/15.
//
#include"planner.h"

std::unique_ptr<minidb::AbstractExecutor> minidb::Planner::Plan(SQLStatement *ast)  {
    if (ast->GetType() == StatementType::SELECT) {
        return PlanSelect(static_cast<SelectStatement*>(ast));
    } else if (ast->GetType() == StatementType::INSERT) {
        return PlanInsert(static_cast<InsertStatement*>(ast));
    }else if (ast->GetType()==StatementType::DELETE) {
        return PlanDelete(static_cast<DeleteStatement *>(ast));
    }else if (ast->GetType()==StatementType::UPDATE)
        return PlanUpdate(static_cast<UpdateStatement *>(ast));
    throw std::runtime_error("Planner Error: Unsupported AST node.");
}

std::unique_ptr<minidb::AbstractExecutor> minidb::Planner::PlanSelect(SelectStatement *stmt) {
    TableHeap *table = catalog_->GetTableHeap(stmt->table_name_);
    const std::vector<TypeId>&type_ids=catalog_->GetSchema(stmt->table_name_).GetSchemaType();
    std::unique_ptr<AbstractExecutor> child=std::make_unique<SeqScanExecutor>(table, type_ids);
    if (stmt->cond_) {
        return std::make_unique<FilterExecutor>(child.release(),stmt->cond_.get(),&catalog_->GetSchema(stmt->table_name_));
    }
    return child;
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

std::unique_ptr<minidb::AbstractExecutor> minidb::Planner::PlanDelete(DeleteStatement *stmt) {
    TableHeap *tb_hp = catalog_->GetTableHeap(stmt->table_name_);
    const std::vector<TypeId>& type_ids = catalog_->GetSchema(stmt->table_name_).GetSchemaType();
    std::unique_ptr<AbstractExecutor> child = std::make_unique<SeqScanExecutor>(tb_hp, type_ids);
    if (stmt->cond_) {
        child = std::make_unique<FilterExecutor>(child.release(), stmt->cond_.get(), &catalog_->GetSchema(stmt->table_name_));
    }
    return std::make_unique<DeleteExecutor>(child.release(), tb_hp);
}
#include<format>
std::unique_ptr<minidb::AbstractExecutor> minidb::Planner::PlanUpdate(UpdateStatement *stmt) {
    TableHeap *tb_hp = catalog_->GetTableHeap(stmt->table_name_);
    const Schema &schema = catalog_->GetSchema(stmt->table_name_);
    const std::vector<TypeId>& type_ids = schema.GetSchemaType();
    std::unique_ptr<AbstractExecutor> child = std::make_unique<SeqScanExecutor>(tb_hp, type_ids);
    if (stmt->cond_) {
        child = std::make_unique<FilterExecutor>(child.release(), stmt->cond_.get(), &schema);
    }

    std::unordered_map<int32_t ,AbstractExpression*>update_attrs;
    const auto &columns=schema.GetColumns();
    for (const auto &[col_name,expr_ptr]:stmt->updates_) {
        uint32_t col_idx=-1;
        for (uint32_t i=0;i<columns.size();i++) {
            if (columns[i].column_name_==col_name)
                {
                col_idx=i;
                break;
                }
        }
        if (col_idx==-1) {
            throw std::runtime_error(std::format("Planner Error: Column '{}' not found",col_name));
        }
        update_attrs[col_idx] = expr_ptr.get();
    }
    return std::make_unique<UpdateExecutor>(child.release(), tb_hp, &schema, update_attrs);
}
