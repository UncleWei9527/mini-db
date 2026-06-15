//
// Created by wjh on 2026/6/13.
//
#include"table_page.h"
// +---------------------+ <--- 0 字节 (OFFSET_PAGE_ID)
// |   Page ID (4字节)    |
// +---------------------+ <--- 4 字节 (OFFSET_PREV_PAGE_ID)
// | Prev Page ID (4字节) |
// +---------------------+ <--- 8 字节 (OFFSET_NEXT_PAGE_ID)
// | Next Page ID (4字节) | ->
// +---------------------+ <--- 12 字节 (OFFSET_FREE_SPACE)
// | Free Space (4字节)   | ->
// +---------------------+ <--- 16 字节 (OFFSET_NUM_TUPLES)
// | Num Tuples (4字节)   | ->
// +---------------------+ <--- 20 字节 (HEADER_SIZE)
// |   Slot 0 (offset,size)      |
void minidb::TablePage::Init(page_id_t page_id, page_id_t prev_page_id) {
    char*data=page_->GetData();

    std::memcpy(data+OFFSET_PAGE_ID,&page_id,sizeof(page_id));
    std::memcpy(data+OFFSET_PREV_PAGE_ID,&prev_page_id,sizeof(prev_page_id) );
    SetNextPageId(INVALID_PAGE_ID);
    SetFreeSpacePointer(PAGE_SIZE);
    SetNumTuples(0);
}

std::optional<minidb::RID> minidb::TablePage::InsertTuple(const Tuple &tuple) {
    uint32_t new_tuple_num=GetNumTuples()+1;
    uint32_t new_slot_index=new_tuple_num-1;//索引从0开始
    uint32_t tuple_size=tuple.GetStorageSize();
    uint32_t free_space_size=GetFreeSpaceRemaining();
    if (tuple_size+SLOT_SIZE>free_space_size)return std::nullopt;
    uint32_t new_free_pointer=GetFreeSpacePointer()-tuple_size;
    //更新槽数据
    SetNumTuples(new_tuple_num);
    SetSlot(new_slot_index,new_free_pointer,tuple_size);
    SetFreeSpacePointer(new_free_pointer);
    //拷贝tupe数据
    tuple.SerializeTo(page_->GetData()+GetSlotOffset(new_slot_index));
    return RID(page_->GetPageId(),new_slot_index);
}

std::optional<minidb::Tuple> minidb::TablePage::GetTuple(const RID &rid, const std::vector<TypeId> &schema) const {
    if (rid.GetPageId()!=page_->GetPageId()||rid.GetSlotNum()>=GetNumTuples())
        return std::nullopt;
    uint32_t slot_num=rid.GetSlotNum();
    uint32_t slot_data_offset=GetSlotOffset(slot_num);
    uint32_t slot_data_size=GetSlotSize(slot_num);
    if (slot_data_size==0) {
        return std::nullopt;//表示被删除了
    }
    Tuple tuple = Tuple::DeserializeFrom(page_->GetData()+slot_data_offset,schema);
    tuple.SetRid(rid);
    return tuple;
}

bool minidb::TablePage::MarkDelete(const RID &rid) {
    if (rid.GetPageId()!=page_->GetPageId()||rid.GetSlotNum()>=GetNumTuples())
        return false;
    //懒惰删除 记录size为0
    SetSlot(rid.GetSlotNum(),GetSlotOffset(rid.GetSlotNum()),0);
    return true;
}

uint32_t minidb::TablePage::GetFreeSpacePointer() const {
    uint32_t free_space_pointer;
    std::memcpy(&free_space_pointer,page_->GetData()+OFFSET_FREE_SPACE,sizeof(free_space_pointer));
    return free_space_pointer;
}

void minidb::TablePage::SetFreeSpacePointer(uint32_t ptr) {
    std::memcpy(page_->GetData()+OFFSET_FREE_SPACE,&ptr,sizeof(ptr));
}

uint32_t minidb::TablePage::GetNumTuples() const {
    uint32_t tuple_num;
    std::memcpy(&tuple_num,page_->GetData()+OFFSET_NUM_TUPLES,sizeof(tuple_num));
    return tuple_num;
}

void minidb::TablePage::SetNumTuples(uint32_t num) {
    std::memcpy(page_->GetData()+OFFSET_NUM_TUPLES,&num,sizeof(num));
}

void minidb::TablePage::SetSlot(uint32_t slot_num, uint32_t offset, uint32_t size) {
    char*slot_offset=page_->GetData()+HEADER_SIZE+slot_num*SLOT_SIZE;
    std::memcpy(slot_offset,&offset,sizeof(offset));
    std::memcpy(slot_offset+sizeof(offset),&size,sizeof(size));
}

uint32_t minidb::TablePage::GetSlotOffset(uint32_t slot_num) const {
    const char*slot_offset=page_->GetData()+HEADER_SIZE+slot_num*SLOT_SIZE;
    uint32_t offset ;
    std::memcpy( &offset,slot_offset,sizeof(offset));
    return offset;

}

uint32_t minidb::TablePage::GetSlotSize(uint32_t slot_num) const {
    const char*slot_offset=page_->GetData()+HEADER_SIZE+slot_num*SLOT_SIZE;
    uint32_t size ;
    std::memcpy( &size,slot_offset+sizeof(size),sizeof(size));
    return size;
}
//获取剩余空间大小
uint32_t minidb::TablePage::GetFreeSpaceRemaining() const {

    uint32_t free_space=GetFreeSpacePointer();
    return free_space-GetNumTuples()*SLOT_SIZE-HEADER_SIZE;
}

minidb::page_id_t minidb::TablePage::GetNextPageId() const {
    page_id_t next_page_id;
    std::memcpy(&next_page_id,page_->GetData()+OFFSET_NEXT_PAGE_ID,sizeof(page_id_t));
    return next_page_id;
}

void minidb::TablePage::SetNextPageId(page_id_t next_page_id) {
    std::memcpy(page_->GetData()+OFFSET_NEXT_PAGE_ID,&next_page_id,sizeof(page_id_t));
}
