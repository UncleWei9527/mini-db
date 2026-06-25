#include "value.h"
#include<cassert>
#include<string.h>
#include<format>
minidb::Value::Value()
    : value_(std::monostate{}) {
    type_id_ = TypeId::INVALID;
}

minidb::Value::Value(bool value)
    : type_id_(TypeId::BOOLEAN), value_(value) {
}

minidb::Value::Value(int32_t value)
    : type_id_(TypeId::INTEGER), value_(value) {
}

minidb::Value::Value(std::string value)
    : type_id_(TypeId::VARCHAR), value_(std::move(value)) {
}

minidb::Value::Value(const char *value): value_(std::string(value))
{
    type_id_ = TypeId::VARCHAR;
}

bool minidb::Value::CompareEQ(const Value &other) const {
    CheckCompareValid(other);
    return value_==other.value_;
}

bool minidb::Value::CompareNEQ(const Value &other) const {
    CheckCompareValid(other);
    return value_!=other.value_;
}

bool minidb::Value::CompareGT(const Value &other) const {
    CheckCompareValid(other);
    return value_>other.value_;
}

bool minidb::Value::CompareGE(const Value &other) const {
    CheckCompareValid(other);
    return value_>=other.value_;
}

bool minidb::Value::CompareLT(const Value &other) const {
    CheckCompareValid(other);
    return value_<other.value_;
}

bool minidb::Value::CompareLE(const Value &other) const {
    return value_<=other.value_;
}

bool minidb::Value::CheckCompareValid(const Value &other) const {
    if (GetTypeId()!=other.GetTypeId()) {
        throw std::logic_error("Type mismatch in comparison");
        return false;
    }
    return true;
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
    return std::visit([]<typename T>(const T &arg) -> std::string {
        if constexpr (std::is_same_v<T, std::monostate>) {
            return "NULL";
        } else if constexpr (std::is_same_v<T, bool>) {
            return arg ? "true" : "false";
        } else if constexpr (std::is_same_v<T, int32_t>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return std::format("'{}'",arg);
        }else {
            static_assert(sizeof(T)==0,"unknown type");
        }
    }, value_);
}

uint32_t minidb::Value::GetStorageSize() const {
    return std::visit([]<typename T>(const T &arg) ->uint32_t {
        if constexpr (std::is_same_v<T, std::monostate>) {
            return 0;
        } else if constexpr (std::is_same_v<T, bool>) {
            return sizeof(arg);
        } else if constexpr (std::is_same_v<T, int32_t>) {
            return sizeof(arg);

        } else if constexpr (std::is_same_v<T, std::string>) {
            return 4+arg.size();
        }else {
            static_assert(sizeof(T)==0,"unknown type");
        }
    }, value_);
}

uint32_t minidb::Value::SerializeTo(char *storage) const {
    return std::visit([storage]<typename T>(const T &arg) ->uint32_t {
        if constexpr (std::is_same_v<T, std::monostate>) {
            return 0;
        } else if constexpr (std::is_same_v<T, bool>) {
            memcpy(storage,&arg,sizeof(arg));
            return sizeof(arg);
        } else if constexpr (std::is_same_v<T, int32_t>) {
            memcpy(storage,&arg,sizeof(arg));
            return sizeof(arg);

        } else if constexpr (std::is_same_v<T, std::string>) {
            int len=arg.size();
            memcpy(storage,&len,sizeof(len));
            memcpy(storage+sizeof(len),arg.data(),len);
            return 4+len;
        }else {
            static_assert(sizeof(T)==0,"unknown type");
        }
    }, value_);
}

minidb::Value minidb::Value::DeserializeFrom(const char *storage, TypeId type_id, uint32_t *out_bytes_read) {
    switch (type_id) {
        case TypeId::INVALID:
            *out_bytes_read=0;
            return Value();
        case TypeId::BOOLEAN: {
            bool val;
            memcpy(&val,storage,sizeof(bool));
            *out_bytes_read=sizeof(bool);
            return Value(val);
        }
        case TypeId::INTEGER: {
            int32_t val;
            memcpy(&val,storage,sizeof(int32_t));
            *out_bytes_read=sizeof(int32_t);
            return Value(val);
        }
        case TypeId::VARCHAR: {
            int32_t len;
            memcpy(&len,storage,sizeof(int32_t));
            std::string str(storage+sizeof(int32_t),len);
            *out_bytes_read=sizeof(int32_t)+len;
            return Value(str);
        }
        default: {
            throw std::logic_error("unknown type id");
        }
    }
    return Value();
}
