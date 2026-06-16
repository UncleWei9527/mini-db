#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include "table_heap.h"

namespace minidb {
    struct Column {
        std::string column_name_;
        TypeId column_type_;
        Column(std::string name,TypeId type);
    };
    class Schema {
    public:
        explicit Schema(const std::vector<Column>&columns);
        const std::vector<Column>& GetColumns() const { return columns_; }
        const std::vector<TypeId>&GetSchemaType()const{return type_ids_;};
        uint32_t GetColumnCount() const { return columns_.size(); }
    private:
        std::vector<Column>columns_;
        std::vector<TypeId>type_ids_;
    };
    struct TableMetaData {
        TableMetaData(Schema schema, std::unique_ptr<TableHeap> table_heap);
        Schema schema_;
        std::unique_ptr<TableHeap> table_heap_;
    };
    class Catalog {
    public:
        explicit Catalog(BufferPoolManager *bpm);
        void CreateTable(const std::string &table_name, const Schema &schema);
        TableHeap* GetTableHeap(const std::string &table_name) const;
        const Schema& GetSchema(const std::string &table_name) const;
    private:
        BufferPoolManager *bpm_;
        std::unordered_map<std::string, std::unique_ptr<TableMetaData>> tables_;
    };

} // namespace minidb