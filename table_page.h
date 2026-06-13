#pragma once

#include "page.h"
#include "tuple.h"
#include <optional>

namespace minidb {
    class TablePage {
    public:
        // 零拷贝包装：把基础 Page 指针包装起来操作
        explicit TablePage(Page *page) : page_(page) {}

        // 🌟 核心：初始化一个全新的白板页
        void Init(page_id_t page_id, page_id_t prev_page_id = INVALID_PAGE_ID);

        // 🌟 核心：往页里插入一行数据
        // 成功返回它的 RID (包含新生成的 slot_num)。如果这页空间不够了，返回 std::nullopt
        std::optional<RID> InsertTuple(const Tuple &tuple);

        // 🌟 核心：读出一行数据
        std::optional<Tuple> GetTuple(const RID &rid, const std::vector<TypeId> &schema) const;

        // 🌟 核心：逻辑删除一行数据
        bool MarkDelete(const RID &rid);
        uint32_t GetFreeSpaceRemaining() const;
    private:
        Page *page_;

        // ==========================================
        // 内存直接读写助手 (偏移量定义)
        // ==========================================
        static constexpr size_t OFFSET_PAGE_ID = 0;
        static constexpr size_t OFFSET_PREV_PAGE_ID = 4;
        static constexpr size_t OFFSET_NEXT_PAGE_ID = 8;
        static constexpr size_t OFFSET_FREE_SPACE = 12; // 空闲空间指针，刚开始是 4096
        static constexpr size_t OFFSET_NUM_TUPLES = 16; // 槽位数量，刚开始是 0
        static constexpr size_t HEADER_SIZE = 20;
        static constexpr size_t SLOT_SIZE = 8;

        // 为了防止你在 CPP 里写指针强转引发硬件崩溃，我要求你在 CPP 里使用 std::memcpy 实现下面这些私有方法：
        uint32_t GetFreeSpacePointer() const;
        void SetFreeSpacePointer(uint32_t ptr);

        uint32_t GetNumTuples() const;
        void SetNumTuples(uint32_t num);

        // 操作指定的槽位
        void SetSlot(uint32_t slot_num, uint32_t offset, uint32_t size);
        uint32_t GetSlotOffset(uint32_t slot_num) const;
        uint32_t GetSlotSize(uint32_t slot_num) const;
        // 计算公式：空闲空间指针 - (头部大小 + 当前所有槽位占用的大小)

    };

} // namespace minidb