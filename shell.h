#pragma  once
#include<string>
#include "disk_manager.h"
#include "buffer_pool_manager.h"
#include "catalog.h"
#include "token.h"
#include "parser.h"
#include "planner.h"
#include "execution_engine.h"
namespace minidb {
    class Shell {
    public:
        Shell(const std::string&db_file);
        void Run();
    private:
        void ExecuteSQL(const std::string&sql);
    private:
        DiskManager disk_mgr_;
        BufferPoolManager bpm_;
        Catalog catalog_;
        Planner planner_;
        ExecutionEngine exec_engine_;
    };
}