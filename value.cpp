#include "value.h"
#include<cassert>

minidb::Value::Value()
    :value_(std::monostate{})
{
    type_id_ = TypeId::INVALID;
}

minidb::Value::Value(bool value)
    :value_(value)
{
    type_id_ = TypeId::BOOLEAN;
}

minidb::Value::Value(int32_t value)
:value_(value)
{
    type_id_ = TypeId::INTEGER;
}

minidb::Value::Value(std::string value)
    :value_(std::move(value))
{
    type_id_ = TypeId::VARCHAR;
}

minidb::TypeId minidb::Value::GetTypeId() const {
    return type_id_;
}

bool minidb::Value::IsNull() const {
    return std::holds_alternative<std::monostate>(value_);
}

bool minidb::Value::GetAsBool() const {
    if (!std::holds_alternative<bool>(value_)) {
        throw std::logic_error("Type mismatch: Expected BOOL");
    }
    return std::get<bool>(value_);
}

int32_t minidb::Value::GetAsInt() const {
    if (!std::holds_alternative<int32_t>(value_)) {
        throw std::logic_error("Type mismatch: Expected INTEGER");
    }
    return std::get<int32_t>(value_);
}

std::string minidb::Value::GetAsString() const {
    if (!std::holds_alternative<std::string>(value_)) {
        throw std::logic_error("Type mismatch: Expected STRING");
    }
    return std::get<std::string>(value_);
}

std::string minidb::Value::ToString() const {
    return std::visit([]<typename T>(const T& arg) -> std::string {

        if constexpr (std::is_same_v<T, std::monostate>) {
            return "NULL";
        } else if constexpr (std::is_same_v<T, bool>) {
            return arg ? "true" : "false";
        } else if constexpr (std::is_same_v<T, int32_t>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return arg;
        }
    }, value_);
}
