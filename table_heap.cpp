#include "table_heap.h"
#include"cassert"
#include"table_iterator.h"
namespace minidb {
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
        // 1. 尝试上末班车
        Page* page = bpm_->FetchPage(last_page_id_);
        if (page == nullptr) {
            return std::nullopt; // 护栏 1：系统彻底没内存了，直接宣告插入失败
        }

        TablePage tb_page(page);
        std::optional<RID> rid_opt = tb_page.InsertTuple(tuple);

        if (rid_opt.has_value()) {
            bpm_->UnpinPage(page->GetPageId(), true); // 放开老页，已弄脏
            return rid_opt; // 护栏 3：直接返回 opt，不要无脑 .value()
        }

        // 2. 末班车满了，强行造新车厢！
        page_id_t new_page_id;
        Page* new_page = bpm_->NewPage(&new_page_id);

        if (new_page == nullptr) {
            // 护栏 2：致命护栏！造不出新车厢了！
            // 在返回失败之前，必须把刚才捏在手里的老车厢还回去！因为老车厢没写进去，所以是 false (没脏)！
            bpm_->UnpinPage(page->GetPageId(), false);
            return std::nullopt;
        }

        TablePage new_tb_page(new_page);
        new_tb_page.Init(new_page_id, last_page_id_); // 初始化新车厢，指向上一个车厢

        tb_page.SetNextPageId(new_page_id); // 老车厢挂上右挂钩
        bpm_->UnpinPage(page->GetPageId(), true); // 老车厢的挂钩被修改了，弄脏了！释放！

        last_page_id_ = new_page_id; // 记下新的车尾

        // 3. 在新车厢安家
        rid_opt = new_tb_page.InsertTuple(tuple);

        bpm_->UnpinPage(new_page_id, true); // 新车厢写进去了，弄脏了！释放！
        return rid_opt; // 直接返回，如果这数据连全新车厢都塞不下，就顺其自然返回 nullopt
    }

} // namespace minidb