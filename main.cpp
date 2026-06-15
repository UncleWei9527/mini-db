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
#include"table_heap.h"
void TestTableHeap() {
    std::cout << "=== 🚀 开始测试 TableHeap (表堆联动大测试) ===" << std::endl;

    // 1. 搞一个极其恶劣的内存环境：自习室只有 3 个座位！
    DiskManager disk_mgr("test_table_heap.db");
    BufferPoolManager bpm(3, &disk_mgr);

    // 2. 创世：开辟一张表
    TableHeap table(&bpm);
    std::cout << "[1/4] 表堆初始化成功，首尾相连在 Page " << table.GetFirstPageId() << "\n";

    // 3. 暴力测试：疯狂插入 1000 行数据！
    // 我们的每行数据大概 20 多字节，4KB 的页大概能装 100 多行。
    // 1000 行一定会跨越大概 8 到 10 个页！
    std::cout << "[2/4] 开始疯狂插入 1000 条数据...\n";
    std::vector<RID> rids;
    std::vector<TypeId> schema = {TypeId::INTEGER, TypeId::VARCHAR};

    for (int i = 0; i < 1000; i++) {
        Tuple t({Value(i), Value("User_" + std::to_string(i))});
        auto rid_opt = table.InsertTuple(t);
        assert(rid_opt.has_value()); // 必须每一行都能插进去！
        rids.push_back(rid_opt.value());
    }

    std::cout << "[3/4] 1000 条数据插入完毕！最后一条数据的 RID: (页号 "
              << rids.back().GetPageId() << ", 槽号 " << rids.back().GetSlotNum() << ")\n";

    // 4. 精准打击：随机抽取刚才的 RID 进行核对，验证历史页是不是被成功淘汰和重载了！
    std::cout << "[4/4] 正在跨页随机抽查读取数据...\n";

    // 查第 0 条
    auto t_0 = table.GetTuple(rids[0], schema);
    assert(t_0.has_value() && t_0->GetValues()[0].GetAsInt() == 0);

    // 查第 500 条
    auto t_500 = table.GetTuple(rids[500], schema);
    assert(t_500.has_value() && t_500->GetValues()[0].GetAsInt() == 500);

    // 查第 999 条
    auto t_999 = table.GetTuple(rids[999], schema);
    assert(t_999.has_value() && t_999->GetValues()[0].GetAsInt() == 999);

    std::cout << "      抽查全部一致！内存调度完美无缺！\n";
    std::cout << "\n🎉🎉🎉 帅炸了！TableHeap 存储引擎全流程跑通！！！" << std::endl;
}
void TestTableIterator() {
    std::cout << "=== 🚀 开始测试 TableIterator (全表扫描) ===" << std::endl;

    DiskManager disk_mgr("test_iterator.db");
    BufferPoolManager bpm(5, &disk_mgr);
    TableHeap table(&bpm);
    std::vector<TypeId> schema = {TypeId::INTEGER, TypeId::VARCHAR};
    std::vector<RID> rids;

    // 1. 插入 10 条数据
    std::cout << "[1/3] 插入 10 条数据...\n";
    for (int i = 0; i < 10; i++) {
        Tuple t({Value(i), Value("Hero_" + std::to_string(i))});
        rids.push_back(table.InsertTuple(t).value());
    }

    // 2. 模拟真实业务：乱序删掉几个数据，制造墓碑！
    std::cout << "[2/3] 删掉偶数行的数据 (制造空洞)...\n";
    for (int i = 0; i < 10; i += 2) {
        // 先 Fetch 出来改成 Delete，由于我们 TableHeap 没暴露 Delete 接口
        // 我们直接找 TablePage 来删！
        Page* p = bpm.FetchPage(rids[i].GetPageId());
        TablePage tp(p);
        tp.MarkDelete(rids[i]);
        bpm.UnpinPage(rids[i].GetPageId(), true);
    }

    // 3. 见证奇迹：C++ 范围 for 循环扫描全表！
    std::cout << "[3/3] 开始启动检票员，全表扫描...\n";
    std::cout << "------------------------------------\n";

    int scan_count = 0;
    // 💡 看这里！由于我们重载了 *, ++, !=，它用起来和 std::vector 一模一样！
    for (auto it = table.Begin(schema); it != table.End(); ++it) {
        Tuple tuple = *it;
        int id = tuple.GetValues()[0].GetAsInt();
        std::string name = tuple.GetValues()[1].GetAsString();

        std::cout << "扫描到存活数据 -> ID: " << id << " Name: " << name << "\n";
        scan_count++;

        // 断言：扫出来的一定是奇数，因为偶数都被删了！
        assert(id % 2 != 0);
    }
    std::cout << "------------------------------------\n";
    assert(scan_count == 5); // 10 个删了 5 个，必须只能扫出 5 个！

    std::cout << "🎉🎉🎉 无懈可击！TableIterator 全表扫描与墓碑跳过完美通过！！！" << std::endl;
}
int main() {
    // 屏蔽掉之前的测试，专心跑这一关
    // TestBPM();
    // TestTuple();
   // TestTablePage();
    //TestTableHeap();
    TestTableIterator();
    return 0;
}