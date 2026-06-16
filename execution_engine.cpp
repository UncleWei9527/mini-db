//
// Created by wjh on 2026/6/16.
//

#include "execution_engine.h"

std::vector<minidb::Tuple> minidb::ExecutionEngine::Execute(AbstractExecutor *executor) {
    std::vector<Tuple>result_set;
    executor->Init();
    while (auto tuple=executor->Next()) {
        result_set.push_back(tuple.value());
    }
    return result_set;
}
