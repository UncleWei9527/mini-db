#include <iostream>
#include <cassert>
#include <cstring>
#include "disk_manager.h"
#include "buffer_pool_manager.h"

using namespace minidb;

void TestBPM() {
    // 💡 核心设定：我们故意把自习室建得极小，只有 3 个座位！
    // 这样就能轻易触发满载和 LRU 淘汰机制。
    DiskManager disk_mgr("test_bpm.db");
    BufferPoolManager bpm(3, &disk_mgr);

    page_id_t page_id_0, page_id_1, page_id_2, page_id_3;

    // ==========================================
    // 🔪 场景 1：基础分配与内存读写
    // ==========================================
    std::cout << "[1/4] 测试基础分配..." << std::endl;
    Page* p0 = bpm.NewPage(&page_id_0);
    Page* p1 = bpm.NewPage(&page_id_1);
    Page* p2 = bpm.NewPage(&page_id_2);

    // 断言：必须全部分配成功，并且页号应该是 0, 1, 2
    assert(p0 != nullptr && p1 != nullptr && p2 != nullptr);
    assert(page_id_0 == 0 && page_id_1 == 1 && page_id_2 == 2);

    // 在 Page 0 的内存里写点暗号（此时绝对还没写回磁盘）
    std::strcpy(p0->GetData(), "Hello mini_db!");

    // ==========================================
    // 🔪 场景 2：Pin 保护机制测试 (内存爆满极限测试)
    // ==========================================
    std::cout << "[2/4] 测试 Pin 内存保护机制..." << std::endl;
    // 此时 p0, p1, p2 刚被 New 出来，默认 pin_count 都是 1。
    // 自习室（容量3）彻底满了，且没有任何人离开座位（Unpin）。
    // 此时如果再要新的一页，BPM 必须无情拒绝！
    Page* p3_failed = bpm.NewPage(&page_id_3);
    assert(p3_failed == nullptr); // 必须返回 nullptr，绝对不能把 p0/1/2 踢掉！

    // ==========================================
    // 🔪 场景 3：触发 LRU 淘汰与脏页偷偷刷盘
    // ==========================================
    std::cout << "[3/4] 测试脏页刷盘与 LRU 淘汰..." << std::endl;
    // 让 0, 1, 2 号学生离开座位
    bpm.UnpinPage(page_id_0, true);  // ⚠️ 重点：传 true！告诉大管家 p0 被涂改过了，是个脏页！
    bpm.UnpinPage(page_id_1, false); // p1 没改过
    bpm.UnpinPage(page_id_2, false); // p2 没改过

    // 此时再要一页，BPM 会去 LRU 找替死鬼。
    // 因为 0 号最先离开，0 号将被无情淘汰！
    // 💣 核心动作：在淘汰 0 号前，BPM 必须发现它是脏页，并调用 DiskManager 把 "Hello mini_db!" 写入磁盘！
    Page* p3 = bpm.NewPage(&page_id_3);
    assert(p3 != nullptr);
    assert(page_id_3 == 3);

    // 把 p3 也 Unpin 掉
    bpm.UnpinPage(page_id_3, false);

    // ==========================================
    // 🔪 场景 4：从磁盘霸气重载数据 (见证奇迹的时刻)
    // ==========================================
    std::cout << "[4/4] 测试磁盘重载..." << std::endl;
    // p0 早就被踢出内存了，刚才那个座位已经被 p3 霸占了。
    // 现在我们要强制 FetchPage 第 0 页！
    // BPM 必须找一个新座位，然后呼叫 DiskManager 把磁盘里的第 0 页重新读进内存！
    Page* p0_recovered = bpm.FetchPage(page_id_0);
    assert(p0_recovered != nullptr);

    // 激动人心的一刻：看看读回来的数据对不对！
    std::cout << "      恢复出的数据: [" << p0_recovered->GetData() << "]" << std::endl;
    assert(std::strcmp(p0_recovered->GetData(), "Hello mini_db!") == 0);

    bpm.UnpinPage(page_id_0, false);

    std::cout << "\n🎉🎉🎉 太神了！BufferPoolManager 全流程测试通过！" << std::endl;
}
#include"tuple.h"
#include"value.h"
void TestTuple() {
    std::vector<Value> values = { Value(1024), Value("Nova"), Value(true) };
    Tuple t1(values);

    // 2. 准备个内存当盘
    char buffer[256];
    t1.SerializeTo(buffer);

    // 3. 从这块无情冷漠的二进制内存中，复活这行数据！
    std::vector<TypeId> schema = {TypeId::INTEGER, TypeId::VARCHAR, TypeId::BOOLEAN};
    Tuple t2 = Tuple::DeserializeFrom(buffer, schema);
    for (const auto&value:t2.GetValues()) {
        std::cout<<value.ToString()<<std::endl;
    }
}

int main() {
    //TestBPM();
    TestTuple();
}