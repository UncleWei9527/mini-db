#pragma once
#include <cstdint>
#include <cstddef>

namespace minidb {

    // 一页的大小为 4096 字节 (4KB)
    inline constexpr size_t PAGE_SIZE = 4096;

    // 定义页的 ID 类型为 32位整数
    using page_id_t = int32_t;
    using frame_id_t = int32_t;
    // 定义一个特殊的常量，表示无效的页号
    inline constexpr page_id_t INVALID_PAGE_ID = -1;


} // namespace minidb