//
// Created by wjh on 2026/6/15.
//
#include"catalog.h"

void minidb::Catalog::AddTable(const std::string &table_name, TableHeap *table) {
    if (tables_.find(table_name) != tables_.end()) {
        throw std::runtime_error("Catalog Error: Table '" + table_name + "' already exists!");
    }
    tables_[table_name] = table;
}
minidb::TableHeap * minidb::Catalog::GetTable(const std::string &table_name)  {
    if (tables_.find(table_name) == tables_.end()) {
        throw std::runtime_error("Catalog Error: Table '" + table_name + "' not found!");
    }
    return tables_[table_name];
}
