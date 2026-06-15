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
        virtual const std::vector<TypeId>& GetOutputSchema() const = 0;
    };
    class SeqScanExecutor:public AbstractExecutor{
    public:
        SeqScanExecutor(TableHeap*tb_heap,const std::vector<TypeId> &schema);
        void Init()override;
        std::optional<Tuple> Next()override;
        const std::vector<TypeId>& GetOutputSchema() const override ;
    private:
        TableHeap*tb_heap_;
        const std::vector<TypeId> &schema_;
        TableIterator it_;

    };
    class InsertExecutor:public AbstractExecutor {
    public:
        InsertExecutor(AbstractExecutor*child,TableHeap*target_table);
        void Init()override;
        std::optional<Tuple> Next()override;
        const std::vector<TypeId>& GetOutputSchema() const override ;
    private:
        AbstractExecutor *child_;
        TableHeap*target_table_;
        uint32_t insert_count;
        bool has_reported_=false;
        std::vector<TypeId>output_schema_;

    };
}


