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
        explicit TableHeap(BufferPoolManager *bpm);
        explicit TableHeap(BufferPoolManager *bpm,page_id_t first_page_id,page_id_t last_page_id);
        std::optional<RID> InsertTuple(const Tuple &tuple);
        std::optional<Tuple> GetTuple(const RID &rid, const std::vector<TypeId> &schema);
        page_id_t GetFirstPageId() const { return first_page_id_; }
        page_id_t GetLastPageId()const{return last_page_id_;}
        bool MarkDelete(const RID&rid);
        TableIterator Begin(const std::vector<TypeId> &schema);
        TableIterator End();
    private:
        BufferPoolManager *bpm_;
        page_id_t first_page_id_{INVALID_PAGE_ID};
        page_id_t last_page_id_{INVALID_PAGE_ID};
    };

} // namespace minidb