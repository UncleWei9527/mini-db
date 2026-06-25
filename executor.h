//
// Created by wjh on 2026/6/15.
//

#pragma  once
#include"table_heap.h"
#include"tuple.h"
#include"table_iterator.h"
#include"expression.h"
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
        std::optional<Tuple> Next() override ;
    private:
        std::vector<Value> values_;
        bool done_ = false;
    };
    class FilterExecutor:public AbstractExecutor {
    public:
        explicit FilterExecutor(AbstractExecutor*child,AbstractExpression *predicate,const Schema *schema);
        void Init() override;
        std::optional<Tuple> Next() override ;
    private:
        AbstractExecutor*child_;
        AbstractExpression*predicate_;
        const Schema*schema_;

    };
    class DeleteExecutor:public AbstractExecutor {
    public:
        explicit DeleteExecutor(AbstractExecutor*child,TableHeap*target_table);
        void Init() override;
        std::optional<Tuple> Next() override ;
    private:
        AbstractExecutor*child_;
        AbstractExpression*predicate_;
        const Schema*schema_;
        TableHeap*target_table_;
        bool has_report_=false;
    };



}


