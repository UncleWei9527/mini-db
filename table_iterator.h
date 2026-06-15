#pragma once
#include "rid.h"
#include "tuple.h"

namespace minidb {

    class TableHeap; // 前向声明，因为我们要引用大统领

    class TableIterator {
    public:
        // 构造函数：告诉迭代器它归哪个表管，以及当前站在哪个 RID 上
        TableIterator(TableHeap *table_heap, RID rid, const std::vector<TypeId> &schema);

        // 🌟 核心 1：解引用运算符 (*it)。获取当前游标指向的 Tuple！
        Tuple operator*();

        // 🌟 核心 2：前置自增运算符 (++it)。走到下一条【活着】的数据！
        TableIterator &operator++();

        // 🌟 核心 3：判等运算符 (it == end)。判断遍历是不是结束了
        bool operator==(const TableIterator &itr) const;
        bool operator!=(const TableIterator &itr) const;

    private:
        TableHeap *table_heap_; // 迭代器需要找表堆要数据
        RID rid_;               // 迭代器当前所在的坐标
        std::vector<TypeId> schema_;
    };

} // namespace minidb