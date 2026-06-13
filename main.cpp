#include <iostream>
#include <cassert>
#include <cstring>
#include "disk_manager.h"
#include "buffer_pool_manager.h"
#include<format>
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
#include"table_page.h"
void TestTablePage() {
    std::cout << "=== 🚀 开始测试 TablePage (分槽页) ===" << std::endl;

    // 1. 搞一块纯净的物理内存页
    Page raw_page;
    TablePage table_page(&raw_page);

    // 初始化为第 8 号数据页
    table_page.Init(8);
    std::cout << "[1/5] 初始化成功，当前剩余空间：" << table_page.GetFreeSpaceRemaining() << " 字节\n";

    // 2. 准备 Schema 和测试数据
    std::vector<TypeId> schema = {TypeId::INTEGER, TypeId::VARCHAR, TypeId::BOOLEAN};
    Tuple t1({Value(1), Value("Alice"), Value(true)});
    Tuple t2({Value(2), Value("Bob_Super_Long_Name"), Value(false)});

    // 3. 测试插入数据
    auto rid1_opt = table_page.InsertTuple(t1);
    assert(rid1_opt.has_value());
    RID rid1 = rid1_opt.value();
    std::cout << "[2/5] 插入 Alice 成功！RID: (页号 " << rid1.GetPageId() << ", 槽号 " << rid1.GetSlotNum() << ")\n";

    auto rid2_opt = table_page.InsertTuple(t2);
    assert(rid2_opt.has_value());
    RID rid2 = rid2_opt.value();
    std::cout << "      插入 Bob 成功！RID: (页号 " << rid2.GetPageId() << ", 槽号 " << rid2.GetSlotNum() << ")\n";

    // 4. 测试精准读取
    auto fetch_t1 = table_page.GetTuple(rid1, schema);
    assert(fetch_t1.has_value());
    assert(fetch_t1->GetValues()[1].GetAsString() == "Alice");
    std::cout << "[3/5] 根据 RID 读取 Alice 成功，数据完全一致！\n";

    // 5. 测试墓碑机制 (逻辑删除)
    std::cout << "[4/5] 正在对 Alice 执行逻辑删除 (立墓碑)...\n";
    bool delete_res = table_page.MarkDelete(rid1);
    assert(delete_res == true);

    // Alice 应该读不到了
    auto fetch_deleted = table_page.GetTuple(rid1, schema);
    assert(!fetch_deleted.has_value());

    // Bob 应该不受任何影响
    auto fetch_t2 = table_page.GetTuple(rid2, schema);
    assert(fetch_t2.has_value());
    assert(fetch_t2->GetValues()[1].GetAsString() == "Bob_Super_Long_Name");
    std::cout << "      验证通过：Alice 已被删除，Bob 依然存活，且物理游标完美解耦！\n";

    // 6. 测试极限打爆内存
    std::cout << "[5/5] 正在测试空间耗尽拦截...\n";
    Tuple giant_tuple({Value(999), Value(std::string(4000, 'X')), Value(false)});
    auto giant_opt = table_page.InsertTuple(giant_tuple);
    assert(!giant_opt.has_value()); // 空间不够，必须安全拒绝！
    std::cout << std::format("      空间耗尽检测成功，超大 Tuple 被完美拦截！{}\n",table_page.GetFreeSpaceRemaining());

    std::cout << "\n🎉🎉🎉 太神了！TablePage 分槽页测试完美通过！" << std::endl;
}

int main() {
    // 屏蔽掉之前的测试，专心跑这一关
    // TestBPM();
    // TestTuple();
    TestTablePage();
    return 0;
}