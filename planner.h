#pragma once
#include <memory>
#include "parser.h"
#include "executor.h"
#include "catalog.h"

namespace minidb {

    class Planner {
    public:
        explicit Planner(Catalog *catalog) : catalog_(catalog) {}
        std::unique_ptr<AbstractExecutor> Plan(SQLStatement *ast);
    private:
        Catalog *catalog_;

        std::unique_ptr<AbstractExecutor> PlanSelect(SelectStatement *stmt) ;
        std::unique_ptr<AbstractExecutor> PlanInsert(InsertStatement *stmt) ;
    };

} // namespace minidb