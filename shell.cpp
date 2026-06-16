//
// Created by wjh on 2026/6/16.
//
#include"shell.h"

minidb::Shell::Shell(const std::string &db_file) : disk_mgr_(db_file),
          bpm_(10, &disk_mgr_),
          catalog_(&bpm_),
          planner_(&catalog_)
{
    std::cout << "mini_db 启动成功！(数据文件: " << db_file << ")\n";
    std::cout << "输入 SQL 语句以执行，输入 'exit' 或 'quit' 退出。\n";
}

void minidb::Shell::Run() {
    std::string sql;
    while (true) {
        std::cout<<"minidb> ";
        std::getline(std::cin,sql);
        if (sql=="exit"||sql=="quit") {
            break;
        }
        if (sql.empty()) {
            continue;
        }
        ExecuteSQL(sql);

    }
}

void minidb::Shell::ExecuteSQL(const std::string &sql) {
    try {
        Tokenizer tokenizer(sql);
        Parser parser(tokenizer.Tokenize());
        auto ast=parser.Parse();
        if (ast->GetType() == StatementType::CREATE_TABLE) {
            // 执行 DDL: 建表
            auto create_stmt = static_cast<CreateTableStatement*>(ast.get());
            Schema new_schema(create_stmt->columns_);
            catalog_.CreateTable(create_stmt->table_name_, new_schema);
            std::cout << "Query OK, 0 rows affected.\n";

        } else {
            auto executor = planner_.Plan(ast.get());
            auto result_set = exec_engine_.Execute(executor.get());

            if (ast->GetType() == StatementType::INSERT) {
                std::cout << "Query OK, " << result_set[0].GetValues()[0].GetAsInt() << " rows affected.\n";
            } else if (ast->GetType() == StatementType::SELECT) {
                for (const auto& tuple : result_set) {
                    std::cout << tuple.ToString() << "\n";
                }
                std::cout << result_set.size() << " rows in set.\n";
            }
        }
    }catch (const std::exception& e) {
        std::cout << "ERROR: " << e.what() << "\n";
    }
}
