#pragma once

#include <fstream>
#include <string>
#include <span>
#include "config.h"

namespace minidb {

    class DiskManager {
    public:
        // 构造函数：打开或创建一个指定名字的数据库文件（例如 "mini_db.db"）
        explicit DiskManager(const std::string &db_file);
        ~DiskManager();
        DiskManager(const DiskManager&) = delete;
        DiskManager& operator=(const DiskManager&) = delete;
        void WritePage(page_id_t page_id, std::span<const char> page_data);
        void ReadPage(page_id_t page_id, std::span<char> page_data);
        page_id_t AllocatePage();

    private:
        std::string file_name_;
        std::fstream db_io_;
        page_id_t next_page_id_{0};
    };


} // namespace minidb