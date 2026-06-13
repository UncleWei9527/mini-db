//
// Created by wjh on 2026/6/13.
//
#include"tuple.h"
uint32_t minidb::Tuple::GetStorageSize() const {
    uint32_t total_size=0;
    for (const auto &value:values_) {
        total_size+=value.GetStorageSize();
    }
    return total_size;
}

void minidb::Tuple::SerializeTo(char *storage) const {
    uint32_t offset=0;
    for (const auto &value:values_) {
        offset+=value.SerializeTo(storage+offset);
    }

}

minidb::Tuple minidb::Tuple::DeserializeFrom(const char *storage, const std::vector<minidb::TypeId> &schema) {
    std::vector<Value>values;
    uint32_t offset=0;
    for (auto type_id:schema) {
        uint32_t read_bytes;
        values.push_back(Value::DeserializeFrom(storage+offset,type_id,&read_bytes));
        offset+=read_bytes;
    }
    return Tuple(values);
}
