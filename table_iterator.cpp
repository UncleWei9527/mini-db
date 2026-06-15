#include"table_iterator.h"
#include"table_heap.h"
namespace minidb {
    TableIterator::TableIterator(TableHeap *table_heap, RID rid, const std::vector<TypeId> &schema)
    : table_heap_(table_heap), rid_(rid), schema_(schema) {}

    Tuple TableIterator::operator*() {
        if (rid_.GetPageId()==INVALID_PAGE_ID) {
            throw std::logic_error("Iterator out of bounds");
        }
        auto tuple_opt=table_heap_->GetTuple(rid_,schema_);
        return tuple_opt.value();
    }

    TableIterator & TableIterator::operator++() {
        BufferPoolManager*bpm=table_heap_->bpm_;
        page_id_t curr_page_id=rid_.GetPageId();
        uint32_t curr_slot_num=rid_.GetSlotNum();
        Page*page=bpm->FetchPage(curr_page_id);
        bool is_first_search=true;
        while (page) {
            TablePage tb_page(page);
            uint32_t start_slot=is_first_search?curr_slot_num+1:0;
            is_first_search=false;
            for (uint32_t i=start_slot;i<tb_page.GetNumTuples();i++) {
                if (tb_page.GetSlotSize(i)>0) {
                    rid_.Set(curr_page_id,i);
                    bpm->UnpinPage(curr_page_id,false);
                    return *this;
                }
            }
            page_id_t next_page_id = tb_page.GetNextPageId();
            bpm->UnpinPage(curr_page_id, false);
            if (next_page_id == INVALID_PAGE_ID) {
                rid_.Set(INVALID_PAGE_ID, 0);
                return *this;
            }
            curr_page_id = next_page_id;
            page = bpm->FetchPage(curr_page_id);

        }
        return *this;
    }

    bool TableIterator::operator==(const TableIterator &itr) const {
        return rid_==itr.rid_;
    }

    bool TableIterator::operator!=(const TableIterator &itr) const {
        return rid_!=itr.rid_;
    }

}
