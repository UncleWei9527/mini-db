//
// Created by wjh on 2026/6/15.
//
#include"catalog.h"

minidb::TableMetaData::TableMetaData(Schema schema, std::unique_ptr<TableHeap> table_heap)
: schema_(std::move(schema)), table_heap_(std::move(table_heap))
{
}

minidb::Catalog::Catalog(BufferPoolManager *bpm)
    :bpm_(bpm)
{
}

void minidb::Catalog::CreateTable(const std::string &table_name, const Schema &schema) {
    if (tables_.find(table_name) != tables_.end()) {
        throw std::runtime_error("Catalog Error: Table '" + table_name + "' already exists!");
    }
    auto table_heap = std::make_unique<TableHeap>(bpm_);
    tables_[table_name] = std::make_unique<TableMetaData>(schema, std::move(table_heap));
}

minidb::TableHeap * minidb::Catalog::GetTableHeap(const std::string &table_name) const {
    if (tables_.find(table_name) == tables_.end()) {
        throw std::runtime_error("Catalog Error: Table '" + table_name + "' not found!");
    }
    return tables_.at(table_name)->table_heap_.get();
}

const minidb::Schema & minidb::Catalog::GetSchema(const std::string &table_name) const {
    if (tables_.find(table_name) == tables_.end()) {
        throw std::runtime_error("Catalog Error: Table '" + table_name + "' not found!");
    }
    return tables_.at(table_name)->schema_;
}
minidb::Column::Column(std::string name, TypeId type)
    :column_name_(name),column_type_(type)
{

}

minidb::Schema::Schema(const std::vector<Column> &columns)
:columns_(columns)
{
    for (const auto&col:columns) {
        type_ids_.push_back(col.column_type_);
    }
}
