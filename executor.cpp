//
// Created by wjh on 2026/6/15.
//

#include "executor.h"

minidb::SeqScanExecutor::SeqScanExecutor(TableHeap *tb_heap,std::vector<TypeId> schema)
    :it_(tb_heap->End()),schema_(std::move(schema)),tb_heap_(tb_heap)
{
}

void minidb::SeqScanExecutor::Init() {
    it_=tb_heap_->Begin(schema_);
}

std::optional<minidb::Tuple> minidb::SeqScanExecutor::Next() {

    if (it_!=tb_heap_->End()) {
        auto temp=it_;
        ++it_;
        return *temp;
    }
    return std::nullopt;
}


minidb::InsertExecutor::InsertExecutor(AbstractExecutor *child, TableHeap *target_table)
    :child_(child),target_table_(target_table)
{

}

void minidb::InsertExecutor::Init() {
    child_->Init();
    has_reported_=false;
    insert_count=0;
}

std::optional<minidb::Tuple> minidb::InsertExecutor::Next() {
    if (has_reported_) {
        return std::nullopt;
    }
    while (true) {
        std::optional<Tuple>tuple_from_child=child_->Next();
        if (!tuple_from_child.has_value()) {
            break;
        }
        target_table_->InsertTuple(tuple_from_child.value());
        insert_count++;
    }
    std::vector<Value>report_value;
    report_value.push_back(Value(static_cast<int32_t>(insert_count)));
    Tuple report_tuple(report_value);
    has_reported_=true;
    return report_tuple;

}


minidb::ValuesExecutor::ValuesExecutor(const std::vector<Value> &values)
    :values_(values)
{

}

void minidb::ValuesExecutor::Init() {
    done_ = false; // 状态复位
}

std::optional<minidb::Tuple> minidb::ValuesExecutor::Next() {
    if (done_) {
        return std::nullopt; // 已经吐过一行了，结束
    }
    done_ = true;
    return Tuple(values_);
}

