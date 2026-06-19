#include"buffer_pool_manager.h"

#include <algorithm>

minidb::BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager *disk_manager)
    :disk_manager_(disk_manager),replacer_(pool_size),pages_(pool_size),pool_size_(pool_size)
{
    for (frame_id_t i=0;i<pool_size;i++)
        free_list_.push_back(i);
}

minidb::BufferPoolManager::~BufferPoolManager() {
    //把脏页写回
    for (auto it=page_table_.begin();it!=page_table_.end();it++) {
        page_id_t page_id=it->first;
        frame_id_t frame_id=it->second;
        if (pages_[frame_id].IsDirty()) {
            FlushPage(page_id);
        }
    }
}
//获取页
minidb::Page * minidb::BufferPoolManager::FetchPage(page_id_t page_id) {

    //已经在缓存里面
    auto it=page_table_.find(page_id);
    if (it!=page_table_.end()) {
        //更新lru的优先级
        Page&page= pages_[it->second];
        page.pin_count_++;//有人占用，计数+1
        return &page;
    }
    //重新去读取磁盘申请 如果有不用的内存帧 我们删除，并让新的页占用
    std::optional<frame_id_t> opt_frame_id=FindVictimOrFreeFrame();
    if (opt_frame_id.has_value()) {
        frame_id_t frame_id =opt_frame_id.value();
        Page &page = pages_[frame_id];
        disk_manager_->ReadPage(page_id,
            std::span<char>(page.GetData(),PAGE_SIZE));
        page_table_[page_id]=frame_id;
        page.pin_count_=1;//引用次数为1
        page.is_dirty_=false;
        page.page_id_=page_id;
        //将这个帧从删除名单移除
        replacer_.Pin(frame_id);
        return &page;
    }
    return nullptr;

}
//申请新的页
minidb::Page * minidb::BufferPoolManager::NewPage(page_id_t *out_page_id) {
    //查找是否有空闲帧 ->(有剩余的内存)->物理开辟页
    std::optional<frame_id_t> opt_frame = FindVictimOrFreeFrame();
    if (!opt_frame.has_value()) {
        return nullptr; // 内存爆满
    }
    frame_id_t frame_id = opt_frame.value();
    Page &page = pages_[frame_id];

    page_id_t new_page_id = disk_manager_->AllocatePage();
    *out_page_id = new_page_id;
    page.ResetMemory();

    page_table_[new_page_id] = frame_id;
    page.page_id_ = new_page_id;
    page.pin_count_ = 1;
    page.is_dirty_ = false;

    replacer_.Pin(frame_id);

    return &page;
}
//标记某页不被使用了
bool minidb::BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty) {
    auto it=page_table_.find(page_id);
    if (it==page_table_.end())return false;
    frame_id_t frame_id=page_table_[page_id];
    Page&page= pages_[frame_id];
    if (page.pin_count_<=0)
        return false;
    page.pin_count_--;
    if (is_dirty)page.is_dirty_=true;
    if (page.pin_count_==0) {
        replacer_.Unpin(frame_id);
    }
    return true;
}
//将页更新到磁盘中
bool minidb::BufferPoolManager::FlushPage(page_id_t page_id) {
    auto it=page_table_.find(page_id);
    if (it==page_table_.end())return false;
    frame_id_t frame_id=it->second;
    Page&page=pages_[frame_id];
    page.is_dirty_=false;
    disk_manager_->WritePage( page_id,std::span<const char>(page.GetData(), PAGE_SIZE));
    return true;
}

uint32_t minidb::BufferPoolManager::GetPageNumber() const {
    return disk_manager_->GetPageNumber();
}

//查找空余的内存帧 ，如果没有lru替换
std::optional<minidb::frame_id_t> minidb::BufferPoolManager::FindVictimOrFreeFrame() {
    //判断我们物理缓存是否还有空间 没有的话要排除一帧
    if (free_list_.size()) {
        frame_id_t free_frame=free_list_.front();
        free_list_.pop_front();
        return free_frame;
    }
    auto victim=replacer_.Victim();
    if (!victim.has_value())return std::nullopt;
    frame_id_t frame_id=victim.value();
    Page&victim_page=pages_[frame_id];
    //脏页写入磁盘
    if (victim_page.IsDirty()) {
        disk_manager_->WritePage(victim_page.GetPageId(),
            std::span<const char>(victim_page.GetData(),PAGE_SIZE));
    }
    //更新页表
    page_table_.erase(victim_page.GetPageId());
    return frame_id;
}
