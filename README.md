# 🚀 mini_db

`mini_db` 是一个从零手写、基于 C++20 的关系型数据库内核。


## 🛠️ 技术栈与环境

* **开发语言**: C++20
* **构建系统**: CMake
* **单元测试**: Google Test (GTest)
* **推荐环境**: Ubuntu 25 

## 🎯 目标 (Roadmap)

实现一个带有 B+ 树索引和火山执行引擎的数据库内核：

- [ ] **阶段 1：磁盘与持久化 (Disk Manager)**
    - 基于 4KB Page 的空间管理，封装二进制文件 I/O。
- [ ] **阶段 2：缓冲池大管家 (Buffer Pool Manager)**
    - 接管内存，实现 LRU (Least Recently Used) 页面置换算法。
- [ ] **阶段 3：紧凑行存储 (Slotted Page)**
    - 实现“分槽页”架构存储定长与变长行数据（Tuple）。
- [ ] **阶段 4：数据库心脏 (B+ Tree Index)**
    - 实现支持 B+ 树索引结构。
- [ ] **阶段 5：火山执行引擎 (Volcano Engine)**
    - 基于迭代器模式 实现 SeqScan、Insert、Filter 等算子。
- [ ] **阶段 6：交互终端 (REPL & Parser)**
    - 支持基础 SQL 语句的解析

## 📈 当前进度追踪

### ✅类型系统 (Type System)
* 构建`Value` 类。
* 支持 `BOOLEAN`, `INTEGER`, `VARCHAR` 三种基础类型，以及 `NULL`（INVALID）状态。


## 💡 构建与测试
*(待下一阶段完善 CMake 与 GTest 后补充)*