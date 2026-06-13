#pragma once
#include <vector>
#include "rid.h"
#include "value.h"

namespace minidb {

    class Tuple {
    public:
        Tuple() = default;

        // 构造一行数据
        explicit Tuple(std::vector<Value> values) : values_(std::move(values)) {}

        // 获取这行数据的 RID
        RID GetRid() const { return rid_; }
        void SetRid(RID rid) { rid_ = rid; }

        // 获取这行里的所有列数据
        const std::vector<Value>& GetValues() const { return values_; }

        // 🌟 核心：计算整行数据占多大空间 (就是把所有 Value 的 StorageSize 加起来)
        uint32_t GetStorageSize() const;

        // 🌟 核心：序列化整行数据（遍历 values_，挨个调用 Value 的 SerializeTo）
        // 注意：存完一个 Value 后，storage 指针要往前移动对应的字节数！
        void SerializeTo(char *storage) const;

        // 🌟 核心：反序列化整行数据
        // schema 是表结构，告诉我们这行应该有哪几个类型（比如 [INT, VARCHAR, BOOLEAN]）
        static Tuple DeserializeFrom(const char *storage, const std::vector<TypeId> &schema);

    private:
        std::vector<Value> values_;
        RID rid_; // 这行数据在磁盘上的物理位置
    };


} // namespace minidb