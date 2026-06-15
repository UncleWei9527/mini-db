#pragma once
#include "buffer_pool_manager.h"
#include "table_page.h"
#include "tuple.h"
#include <optional>
#include"table_iterator.h"
namespace minidb {

    class TableHeap {
        friend class TableIterator;
    public:
        // 构造函数：开辟一张新表！
        // 它会立刻向 BPM 申请第一页，作为火车的车头。
        explicit TableHeap(BufferPoolManager *bpm);

        // 🌟 核心：往表里无限量插入一行数据
        // 无论跨越多少页，它一定要把数据塞进去（如果满了就造新页），并返回最终安家的 RID
        std::optional<RID> InsertTuple(const Tuple &tuple);

        // 🌟 核心：根据 RID 精准读取某行数据
        std::optional<Tuple> GetTuple(const RID &rid, const std::vector<TypeId> &schema);

        // 获取这列火车的车头（第一页的页号），后面写全表扫描（SELECT *）时要用！
        page_id_t GetFirstPageId() const { return first_page_id_; }
        TableIterator Begin(const std::vector<TypeId> &schema);
        TableIterator End();
    private:
        BufferPoolManager *bpm_;
        page_id_t first_page_id_{INVALID_PAGE_ID}; // 车头的门牌号
        page_id_t last_page_id_{INVALID_PAGE_ID};  // 最后一节车厢的门牌号（插入数据时为了追求 O(1) 光速，直接找尾巴）
    };

} // namespace minidb