#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <stdexcept>
#include "type_id.h"

namespace minidb {

    class Value {
    public:
        // --- 任务 1：构造函数 ---

        // 默认构造，创建一个 NULL 值
        Value();

        // 构造一个布尔值
        explicit Value(bool value);

        // 构造一个整数值
        explicit Value(int32_t value);

        // 构造一个字符串值
        explicit Value(std::string value);
        explicit Value(const char* value);
        // --- 任务 2：基础观察方法 ---

        TypeId GetTypeId() const;
        bool IsNull() const;

        // --- 任务 3：数据提取方法 ---

        // ⚠️ 重点要求：
        // 1. 如果当前的值是 NULL（即 std::monostate），提取任何数据都应抛出异常。
        // 2. 如果你调用的类型不匹配（比如明明是个 INTEGER，却调用了 GetAsString()），
        //    请你在代码里抛出异常：throw std::logic_error("Type mismatch");
        bool GetAsBool() const;
        int32_t GetAsInt() const;
        std::string GetAsString() const;
        std::string ToString()const;
        uint32_t GetStorageSize() const;
        uint32_t SerializeTo(char *storage) const;
        static Value DeserializeFrom(const char *storage, TypeId type_id, uint32_t *out_bytes_read);
    private:
        // 记录当前值在数据库层面是什么类型
        TypeId type_id_;

        // 我们极其安全的变色龙盒子！
        // std::monostate 放在第一位，代表默认的空状态 (NULL)
        std::variant<std::monostate, bool, int32_t, std::string> value_;
    };



} // namespace minidb