#pragma once

#include <list>
#include <mutex>
#include <unordered_map>
#include <optional>
#include "config.h"

namespace minidb {

    class LRUReplacer {
    public:
        explicit LRUReplacer(size_t num_frames);
        ~LRUReplacer() = default;

        // 禁用拷贝
        LRUReplacer(const LRUReplacer&) = delete;
        LRUReplacer& operator=(const LRUReplacer&) = delete;
        std::optional<frame_id_t> Victim();
        void Pin(frame_id_t frame_id);
        void Unpin(frame_id_t frame_id);
        size_t Size() const;

    private:
        size_t capacity_;
        std::list<frame_id_t> lru_list_;
        std::unordered_map<frame_id_t, std::list<frame_id_t>::iterator> lru_hash_;
    };

} // namespace minidb