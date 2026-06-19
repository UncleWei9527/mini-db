#include "disk_manager.h"

minidb::DiskManager::DiskManager(const std::string &db_file)
    :file_name_(db_file)
{
    db_io_.open(file_name_,std::ios::in|std::ios::out|std::ios::binary);
    if (!db_io_.is_open()) {
        db_io_.clear(); // 清理一下 fstream 内部的报错状态
        db_io_.open(db_file, std::ios::out | std::ios::binary);
        db_io_.close();

        // 然后再次以读写模式正常打开！这次肯定成功
        db_io_.open(db_file, std::ios::in | std::ios::out | std::ios::binary);

    }
    db_io_.seekg(0,std::ios::end);
    size_t file_size=db_io_.tellg();
    if (file_size%PAGE_SIZE!=0) {
        throw std::logic_error("file size mod page_size !=0");
    }
    next_page_id_=file_size/PAGE_SIZE;
}

minidb::DiskManager::~DiskManager() {
    db_io_.close();
}

void minidb::DiskManager::WritePage(page_id_t page_id, std::span<const char> page_data) {
    db_io_.seekp (page_id*PAGE_SIZE,std::ios::beg);
    db_io_.write(page_data.data(),page_data.size());
    db_io_.flush();
}

void minidb::DiskManager::ReadPage(page_id_t page_id, std::span<char> page_data) {
    db_io_.seekg(page_id*PAGE_SIZE,std::ios::beg);
    db_io_.read(page_data.data(),PAGE_SIZE);

}

minidb::page_id_t minidb::DiskManager::AllocatePage() {

    return next_page_id_++;
}

uint32_t minidb::DiskManager::GetPageNumber() const {
    return next_page_id_;
}
