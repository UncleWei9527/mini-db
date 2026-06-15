#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include "table_heap.h"

namespace minidb {

    class Catalog {
    public:
        void AddTable(const std::string &table_name, TableHeap *table) ;
        TableHeap* GetTable(const std::string &table_name);

    private:
        std::unordered_map<std::string, TableHeap*> tables_;
    };

} // namespace minidb