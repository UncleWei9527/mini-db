#pragma once

#include <list>
#include <unordered_map>
#include <vector>
#include <optional>
#include <span>
#include "page.h"
#include "disk_manager.h"
#include "lru_replacer.h"

namespace minidb {

    class BufferPoolManager {
    public:
        BufferPoolManager(size_t pool_size, DiskManager *disk_manager);
        ~BufferPoolManager();

        // 禁用拷贝
        BufferPoolManager(const BufferPoolManager&) = delete;
        BufferPoolManager& operator=(const BufferPoolManager&) = delete;
        Page* FetchPage(page_id_t page_id);
        Page* NewPage(page_id_t *out_page_id);
        bool UnpinPage(page_id_t page_id, bool is_dirty);
        bool FlushPage(page_id_t page_id);
        uint32_t GetPageNumber()const;
    private:
        std::optional<frame_id_t> FindVictimOrFreeFrame();

        size_t pool_size_;
        std::vector<Page> pages_;
        DiskManager *disk_manager_;
        LRUReplacer replacer_;

        std::list<frame_id_t> free_list_;
        std::unordered_map<page_id_t, frame_id_t> page_table_;//映射物理页到内存帧
    };

} // namespace minidb