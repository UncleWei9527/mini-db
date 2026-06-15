# mini_db: A Relational Database Engine

`mini_db` 是一个基于 C++ 开发的关系型数据库。本项目旨在实现经典关系型数据库底层的核心基础架构，主要包括磁盘与内存调度、分槽页数据布局以及基于火山模型的查询执行器。

## 架构体系 (Architecture)

系统自底向上由以下核心层级构成：

### 1. 存储引擎层 (Storage Engine)
* **DiskManager**: 负责物理磁盘 I/O，以固定大小的页（Page，默认 4KB）为单位进行二进制读写。
* **BufferPoolManager**: 负责内存页的管理与调度。拦截上层的 Page 请求，维护内存帧（Frames）与物理页的映射；基于 `Pin/Unpin` 机制管理页生命周期，并处理脏页的延迟刷盘。
* **LRUReplacer**: 页面置换组件，配合 BufferPoolManager 工作，在内存不足时采用 LRU 算法驱逐未被 Pin 的页面。

### 2. 数据组织层 (Data Layout)
* **TablePage**: 实现分槽页（Slotted Page）架构，用于在固定 4KB 页面中管理变长记录（Tuple）。头部维护元数据与 Slot 数组，尾部存储实际数据。支持 $O(1)$ 复杂度的逻辑删除。
* **TableHeap**: 表数据的逻辑抽象。通过双向链表管理多个 `TablePage`，对外提供透明的跨页 Tuple 插入、读取以及基于 `TableIterator` 的全表遍历接口。

### 3. 执行引擎层 (Execution Engine)
* **执行模型**: 实现基于 Volcano Model（火山模型）的拉取式执行器。各级算子（Operator）统一实现 `Init()` 与 `Next()` 接口。
* **类型系统**: 封装了 `Value` 类，支持 `INTEGER`, `BOOLEAN`, `VARCHAR` 等基础类型；实现了 `Tuple` 的序列化与反序列化协议，完成 C++ 内存对象与底层连续字节流的转换。

---

## 当前支持的 SQL 语义

基于现有执行算子的组合，目前系统支持以下物理执行流：

✅ **全表扫描**
> SELECT * FROM table_name;

* **执行路径**: `SeqScanExecutor` 直接调用 `TableIterator` 遍历目标 `TableHeap`。

✅ **表间数据导入**
> INSERT INTO table_A SELECT * FROM table_B;

* **执行路径**: `InsertExecutor` 作为父节点嵌套 `SeqScanExecutor`。通过循环调用子节点的 `Next()` 获取数据，并调用目标表的 `TableHeap::InsertTuple()` 完成物理写入。

---

## 演进路线 (Roadmap)

系统基础存储与执行链路已连通，计划实现以下高级特性：

- [ ] **表达式系统 (Expression Evaluation)**: 实现常量、列引用及关系操作符（`=`, `>`, `<`）的动态计算。
- [ ] **条件过滤 (WHERE Clause)**: 引入 `FilterExecutor` 算子，支持带有条件的单表查询过滤。
- [ ] **静态数据插入**: 引入 `ValuesExecutor` 算子，支持单行/多行的原始数据插入语义。
- [ ] **索引引擎 (Index Engine)**: 实现基于磁盘的 B+ 树索引（B+ Tree），支持 $O(\log n)$ 级别的数据检索，并集成 `IndexScan` 算子。
- [ ] **复杂查询算子**: 支持 `Limit`、聚合计算（Aggregation）以及连接（Nested Loop Join）算子。
- [ ] **并发与恢复 (Concurrency & Recovery)**: 引入基于 2PL 的锁管理器与预写式日志（WAL），保障 ACID 特性。