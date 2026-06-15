//
// Created by wjh on 2026/6/15.
//

#pragma  once
#include"table_heap.h"
#include"tuple.h"
namespace minidb {
    class AbstractExecutor {
        virtual void Init()=0;
        virtual Tuple Next()=0;
    };
    class 
}


#endif //DB_ABSTRACT_EXECUTOR_H
