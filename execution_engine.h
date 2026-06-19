#pragma once
#include <vector>
#include <memory>
#include"executor.h"
#include"tuple.h"
namespace minidb {
    class ExecutionEngine {
    public:
        std::vector<Tuple>Execute(AbstractExecutor*executor);
    };
}

