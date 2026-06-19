#include "table_heap.h"
#include"cassert"
#include"table_iterator.h"
namespace minidb {
    //创造一个表
    TableHeap::TableHeap(BufferPoolManager *bpm) : bpm_(bpm) {
        Page*new_page=bpm->NewPage(&first_page_id_);
        if (!new_page) {
            throw std::logic_error("bpm newpage return nullptr");
        }
        last_page_id_=first_page_id_;
        TablePage first_tb_page(new_page);
        first_tb_page.Init(first_page_id_,INVALID_PAGE_ID);
        bpm_->UnpinPage(first_page_id_,true);
    }
    //从文件中序列一个表
    TableHeap::TableHeap(BufferPoolManager *bpm,page_id_t first_page_id,page_id_t last_page_id)
        :first_page_id_(first_page_id),last_page_id_(last_page_id),bpm_(bpm)
    {
    }

    // 读取数据
    std::optional<Tuple> TableHeap::GetTuple(const RID &rid, const std::vector<TypeId> &schema) {
        Page*page=bpm_->FetchPage(rid.GetPageId());
        if (page) {
            TablePage tb_page(page);
            std::optional<Tuple>result_tuple=tb_page.GetTuple(rid,schema);
            bpm_->UnpinPage(rid.GetPageId(),false);
            return result_tuple;
        }
        return std::nullopt;
    }

    TableIterator TableHeap::End() {
        // 永远用 INVALID_PAGE_ID 代表宇宙的尽头
        return TableIterator(this, RID(INVALID_PAGE_ID, 0),{});
    }

    TableIterator TableHeap::Begin(const std::vector<TypeId> &schema) {
        page_id_t curr_page_id = first_page_id_;
        if (curr_page_id == INVALID_PAGE_ID) {
            return End();
        }
        Page *page = bpm_->FetchPage(curr_page_id);
        while (page != nullptr) {
            TablePage table_page(page);

            for (uint32_t i = 0; i < table_page.GetNumTuples(); i++) {
                if (table_page.GetSlotSize(i) > 0) {
                    bpm_->UnpinPage(curr_page_id, false);
                    return TableIterator(this, RID(curr_page_id, i), schema);
                }
            }

            page_id_t next_page_id = table_page.GetNextPageId();
            bpm_->UnpinPage(curr_page_id, false);

            curr_page_id = next_page_id;
            if (curr_page_id != INVALID_PAGE_ID) {
                page = bpm_->FetchPage(curr_page_id);
            } else {
                page = nullptr;
            }
        }

        return End();
    }

    std::optional<RID> TableHeap::InsertTuple(const Tuple &tuple) {
        Page* page = bpm_->FetchPage(last_page_id_);
        if (page == nullptr) {
            return std::nullopt; // 护栏 1：系统彻底没内存了，直接宣告插入失败
        }

        TablePage tb_page(page);
        std::optional<RID> rid_opt = tb_page.InsertTuple(tuple);

        if (rid_opt.has_value()) {
            bpm_->UnpinPage(page->GetPageId(), true);
            return rid_opt;
        }

        page_id_t new_page_id;
        Page* new_page = bpm_->NewPage(&new_page_id);

        if (new_page == nullptr) {
            bpm_->UnpinPage(page->GetPageId(), false);
            return std::nullopt;
        }

        TablePage new_tb_page(new_page);
        new_tb_page.Init(new_page_id, last_page_id_);

        tb_page.SetNextPageId(new_page_id);
        bpm_->UnpinPage(page->GetPageId(), true);

        last_page_id_ = new_page_id;

        rid_opt = new_tb_page.InsertTuple(tuple);

        bpm_->UnpinPage(new_page_id, true);
        return rid_opt;
    }

} // namespace minidb