cmake_minimum_required(VERSION 4.2)
project(db)

set(CMAKE_CXX_STANDARD 20)
add_executable(db main.cpp
        type_id.h
        value.cpp
        value.h
        config.h
        disk_manager.cpp
        disk_manager.h
        lru_replacer.h
        lru_replacer.cpp
        page.h
        buffer_pool_manager.h
        buffer_pool_manager.cpp
        rid.h
        tuple.h
        tuple.cpp
        table_page.h
        table_page.cpp
        table_heap.h
        table_heap.cpp
        table_iterator.h
        table_iterator.cpp
        executor.cpp
        executor.h
        token.h
        token.cpp
        Parser.h
        Parser.cpp)
