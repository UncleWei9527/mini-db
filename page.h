#pragma once
#include <cstring>
#include "config.h"

namespace minidb {

    class Page {
        friend class BufferPoolManager; // 允许大管家随便修改我的私有变量
    public:
        Page() { ResetMemory(); }
        ~Page() = default;

        // 获取这页内部 4KB 数据的指针，给执行引擎读写用的
        char* GetData() { return data_; }
        // 获取这页目前装的是硬盘上的哪一页
        page_id_t GetPageId() const { return page_id_; }
        // 有几个线程正在看这页
        int GetPinCount() const { return pin_count_; }
        // 是否是被修改过的脏页
        bool IsDirty() const { return is_dirty_; }

    private:
        void ResetMemory() {
            std::memset(data_, 0, PAGE_SIZE);
            page_id_ = INVALID_PAGE_ID;
            pin_count_ = 0;
            is_dirty_ = false;
        }

        char data_[PAGE_SIZE];            // 内存数据！
        page_id_t page_id_;               //  哪一页
        int pin_count_;                   // 引用计数 (0 代表没人在看，可以被 LRU 淘汰)
        bool is_dirty_;                   // 脏页标记，如果被改过，淘汰前必须写盘
    };

} // namespace minidb