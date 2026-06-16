//
// Created by wjh on 2026/6/15.
//

#pragma  once
#include"table_heap.h"
#include"tuple.h"
#include"table_iterator.h"
namespace minidb {
    class AbstractExecutor {
    public:
        virtual void Init()=0;
        virtual std::optional<Tuple> Next()=0;
    };
    class SeqScanExecutor:public AbstractExecutor{
    public:
        SeqScanExecutor(TableHeap*tb_heap,std::vector<TypeId> schema);
        void Init()override;
        std::optional<Tuple> Next()override;
    private:
        TableHeap*tb_heap_;
        std::vector<TypeId> schema_;
        TableIterator it_;

    };
    class InsertExecutor:public AbstractExecutor {
    public:
        InsertExecutor(AbstractExecutor*child,TableHeap*target_table);
        void Init()override;
        std::optional<Tuple> Next()override;
    private:
        AbstractExecutor *child_;
        TableHeap*target_table_;
        uint32_t insert_count;
        bool has_reported_=false;

    };
    class ValuesExecutor:public AbstractExecutor {
    public:
        explicit ValuesExecutor(const std::vector<Value>& values);
        void Init() override;

        // 假设你的 Next 返回的是指针或 optional，这里以返回 std::unique_ptr<Tuple> 为例，
        // 具体返回类型请根据你之前设计的 AbstractExecutor 保持一致。
        std::optional<Tuple> Next() override ;
    private:
        std::vector<Value> values_;
        bool done_ = false;
    };
}


