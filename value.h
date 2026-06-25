#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <stdexcept>
#include "type_id.h"

namespace minidb {

    class Value {
    public:
        Value();

        // 构造一个布尔值
        explicit Value(bool value);

        // 构造一个整数值
        explicit Value(int32_t value);

        // 构造一个字符串值
        explicit Value(std::string value);
        explicit Value(const char* value);
        bool CompareEQ(const Value&other)const;
        bool CompareNEQ(const Value&other)const;
        bool CompareGT(const Value&other)const;
        bool CompareGE(const Value&other)const;
        bool CompareLT(const Value&other)const;
        bool CompareLE(const Value&other)const;
        bool CheckCompareValid(const Value&other)const;
        TypeId GetTypeId() const;
        bool IsNull() const;
        bool GetAsBool() const;
        int32_t GetAsInt() const;
        std::string GetAsString() const;
        std::string ToString()const;
        uint32_t GetStorageSize() const;
        uint32_t SerializeTo(char *storage) const;
        static Value DeserializeFrom(const char *storage, TypeId type_id, uint32_t *out_bytes_read);
    private:
        TypeId type_id_;
        std::variant<std::monostate, bool, int32_t, std::string> value_;
    };



} // namespace minidb