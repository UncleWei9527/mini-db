#pragma once

namespace minidb {

    // 数据库支持的类型枚举
    enum class TypeId {
        INVALID = 0, // 表示 NULL 状态
        BOOLEAN,
        INTEGER,
        VARCHAR
    };

} // namespace minidb