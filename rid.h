#pragma once
#include <cstdint>
#include "config.h"

namespace minidb {

    class RID {
    public:
        RID() = default;
        RID(page_id_t page_id, uint32_t slot_num) : page_id_(page_id), slot_num_(slot_num) {}

        page_id_t GetPageId() const { return page_id_; }
        uint32_t GetSlotNum() const { return slot_num_; }

        void Set(page_id_t page_id, uint32_t slot_num) {
            page_id_ = page_id;
            slot_num_ = slot_num;
        }

        // 重载判等运算符 (C++20 可以使用 = default 自动生成！)
        bool operator==(const RID &other) const = default;

    private:
        page_id_t page_id_{INVALID_PAGE_ID};
        uint32_t slot_num_{0};
    };

} // namespace minidb