//
// Created by wjh on 2026/6/12.
//
#include"lru_replacer.h"

#include <stdexcept>
#include<cassert>
minidb::LRUReplacer::LRUReplacer(size_t num_frames) {
    capacity_=num_frames;
}
//删除名单中的最早使用的frame 返回删除的槽(没有nullopt)
std::optional<minidb::frame_id_t> minidb::LRUReplacer::Victim() {
    if (!lru_list_.size())return std::nullopt;

    auto it=lru_hash_.find(lru_list_.back());
    assert(it!=lru_hash_.end());
    lru_hash_.erase(it);
    int num=lru_list_.back();
    lru_list_.pop_back();
    return num;
}
//标记frame为不可删除
void minidb::LRUReplacer::Pin(frame_id_t frame_id) {
    auto it=lru_hash_.find(frame_id);
    if (it!=lru_hash_.end()) {
        lru_list_.erase(it->second);
        lru_hash_.erase(it);
    }
}
//标记frame为可以删除
void minidb::LRUReplacer::Unpin(frame_id_t frame_id) {
    if (Size()==capacity_)return ;
    auto it=lru_hash_.find(frame_id);
    if (it==lru_hash_.end()) {
        lru_list_.push_front(frame_id);
        lru_hash_[frame_id]=lru_list_.begin();
    }
}

size_t minidb::LRUReplacer::Size() const {
    return lru_list_.size();
}
