# **金融交易模拟系统**详细设计说明书（SDD/LLD）

> 版本：1.1.2

---

## 1. 详细设计说明书（SDD / LLD）

### 1.1 系统概述

金融交易模拟系统旨在模拟一个金融平台，支持用户的交易操作，包括股票、期货等资产的买卖。系统提供账户管理、订单管理、撮合执行、历史查询等功能，并支持限价单、市场单等订单类型。

V1.1.2 口径约定：

* 金额统一使用 `Money`（以分为单位，`long long` 存储），数量统一使用 `std::int64_t qty`。
* 模块划分与 `trade_sim` 目录一致：`common` / `model` / `order` / `core` / `io`。
* V1.1.2 为单进程内存版，持久化仅 CSV 文件读写；不包含并发、网络、数据库。

### 1.2 模块设计

#### 1.2.1 账户管理模块（AccountManager）

* **职责**：负责账户创建、查询与存在性检查，并维护账户持仓；提供 CSV 文件读写入口。
* **数据结构**：

  * `Account` 类：包含账户余额与持仓。
  * `AccountManager` 类：管理账户集合。
* **接口设计**：

  * `createAccount(const AccountId& id, Money initial)`: 创建账户。
  * `getAccount(const AccountId& id)`: 获取账户（可提供 const/非 const 重载）。
  * `bool exists(const AccountId& id) const`: 判断账户是否存在。
  * `loadFromFile(const std::string& path)`: 从 CSV 载入账户与持仓（`path` 视为目录，固定文件名为 `accounts.csv` / `positions.csv`）。
  * `saveToFile(const std::string& path) const`: 保存账户与持仓到 CSV（同上）。

#### 1.2.2 订单管理模块（OrderManager）

* **职责**：管理订单生命周期（提交、查询、状态、取消）。
* **数据结构**：

  * `Order` 类：抽象订单基类（Market / Limit）。
  * `OrderManager` 类：持有订单并维护状态。
* **接口设计**：

  * `OrderId nextId() noexcept`: 生成订单 ID。
  * `submit(std::unique_ptr<Order> order)`: 提交订单（转移所有权）。
  * `submitRaw(Order* rawOrder)`: 以裸指针提交（内部接管为 `unique_ptr`）。
  * `Order& get(OrderId id)`: 查询订单。
  * `OrderStatus status(OrderId id) const`: 查询订单状态。
  * `cancel(OrderId id)`: 取消订单。

#### 1.2.3 撮合模块（MatchingEngine）

* **职责**：对输入订单进行撮合，输出成交记录。
* **数据结构**：

  * `Trade` 类：成交记录。
  * `MatchingEngine` 类：撮合引擎。
* **接口设计**：

  * `std::vector<Trade> match(const Order& incoming)`: 撮合订单并返回成交列表。

#### 1.2.4 交易执行模块（TradeExecutor）

* **职责**：业务编排层，负责提交订单、撮合、更新账户、写入历史。
* **数据结构**：

  * `TradeExecutor` 类：组织 AccountManager / OrderManager / MatchingEngine / HistoryManager。
* **接口设计**：

  * `submitAndProcess(std::unique_ptr<Order> order)`: 提交订单并执行。

#### 1.2.5 历史查询模块（HistoryManager）

* **职责**：记录与查询交易历史。
* **数据结构**：

  * `HistoryManager` 类：保存交易记录与索引。
* **接口设计**：

  * `record(const Trade& t, const AccountId& buyer, const AccountId& seller)`: 记录成交。
  * `std::vector<Trade> historyOf(const AccountId& user) const`: 查询用户历史。
  * `loadFromFile(const std::string& path)`: 从 CSV 载入成交记录（`path` 视为目录，固定文件名为 `trades.csv`）。
  * `saveToFile(const std::string& path) const`: 保存成交记录到 CSV（同上）。

#### 1.2.6 文件存储模块（Storage）

* **职责**：提供简单的文本文件读写（CSV）。
* **接口设计**：

  * `std::vector<std::string> readAllLines(const std::string& path)`: 读全量文本。
  * `void writeAllLines(const std::string& path, const std::vector<std::string>& lines)`: 写全量文本。

* **CSV 解析规则**：

  * 无 header
  * 允许空行
  * 不支持注释行
  * 字段不做转义，字段内不可包含逗号
  * 解析失败：整文件失败并抛 `ParseErrorException`

### 1.3 关键类设计

#### 1.3.1 Money 与 Position

* **Money**：以分为单位的金额类型，内部使用 `long long` 存储。
* **FromYuan**：`cents = llround(yuan * 100.0)`（四舍五入，远离 0）。
* **Position**：持仓结构，包含 `symbol` 与 `std::int64_t qty`。

#### 1.3.2 Account 类

* **成员变量**：

  * `AccountId userId`: 用户 ID。
  * `Money balance`: 账户余额。
  * `std::unordered_map<Symbol, std::int64_t> positions`: 用户持仓。

* **内存管理**：

  * 账户与持仓使用标准容器管理内存。

#### 1.3.3 Order 类

* **类层次**：

  * `Order`：抽象基类，字段 `id` / `user` / `symbol` / `side` / `qty`，提供 `OrderKind kind()` 与 `Money limitPrice()`。
  * `MarketOrder`：无价格，`limitPrice()` 返回 0。
  * `LimitOrder`：持有限价 `Money limit`，`limitPrice()` 返回该值。
* **约束**：

  * `qty > 0`，`symbol` 非空。

#### 1.3.4 Trade 类

* **成员变量**：

  * `TradeId tradeId`: 成交 ID。
  * `OrderId buyOrderId`: 买单 ID。
  * `OrderId sellOrderId`: 卖单 ID。
  * `Symbol symbol`: 成交标的。
  * `std::int64_t qty`: 成交数量。
  * `Money price`: 成交价格。

* **内存管理**：

  * 成交记录存储在 `std::vector` 中，由容器管理内存。

### 1.4 内存管理

* **账户管理**：账户与持仓使用标准容器（如 `std::unordered_map`）管理。
* **订单管理**：订单对象由 `std::unique_ptr` 管理，避免内存泄漏。
* **交易执行**：成交记录由 `HistoryManager` 持有，容器负责释放。

### 1.5 异常处理

* **参数无效**：抛出 `InvalidArgumentException`。
* **资源未找到**：抛出 `NotFoundException`。
* **重复资源**：抛出 `TradeSimException(ErrorCode::Duplicate)`。
* **资金不足**：抛出 `InsufficientFundsException`。
* **持仓不足**：`Account::addPosition` 抛 `TradeSimException(ErrorCode::InsufficientPosition, ...)`。
* **IO 与解析错误**：抛出 `IOErrorException` / `ParseErrorException`。
* **非法状态**：抛出 `TradeSimException(ErrorCode::InvalidState)`。

---

### 1.6 未来扩展（V1.1.2 不包含）

* **并发与线程安全**：多线程撮合、锁与一致性保证。
* **网络与远程接口**：服务化、RPC/HTTP 接入。
* **数据库持久化**：替换 CSV，支持事务与恢复。
* **高级撮合与订单簿**：撮合优先级、部分成交与挂单簿。

