# **金融交易模拟系统**详细设计说明书（SDD/LLD）

> 版本：1.2

---

## 1. 详细设计说明书（SDD / LLD）

### 1.1 系统概述

金融交易模拟系统旨在模拟一个金融平台，支持用户的交易操作，包括股票、期货等资产的买卖。系统提供账户管理、订单管理、撮合执行、历史查询等功能，并支持限价单、市场单等订单类型。

V1.2 口径约定：

* 金额统一使用 `Money`（以分为单位，`long long` 存储），数量统一使用 `std::int64_t qty`。
* 模块划分与 `trade_sim` 目录一致：`common` / `model` / `order` / `core` / `io`。
* V1.2 为单进程内存版，持久化仅 CSV 文件读写；不包含并发、网络、数据库。

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

  * 交易记录由 `HistoryManager` 内部的 `unordered_map<TradeId, Trade>` 保存，并通过 `unordered_map<AccountId, vector<TradeId>>` 建索引用于按用户查询。

### 1.4 内存管理

* **账户管理**：账户与持仓使用标准容器（如 `std::unordered_map`）管理。
* **订单管理**：订单对象由 `std::unique_ptr` 管理，避免内存泄漏。
* **交易执行**：成交记录由 `HistoryManager` 持有，容器负责释放。

### 1.5 异常处理

* **参数无效**：抛出 `InvalidArgumentException`。
* **资源未找到**：抛出 `NotFoundException`。
* **重复资源**：抛出 `TradeSimException(ErrorCode::Duplicate, ...)`。
* **资金不足**：抛出 `InsufficientFundsException`。
* **持仓不足**：`Account::addPosition` 抛 `TradeSimException(ErrorCode::InsufficientPosition, ...)`。
* **IO 与解析错误**：抛出 `IOErrorException` / `ParseErrorException`。
* **非法状态**：抛出 `TradeSimException(ErrorCode::InvalidState)`。

---

### 1.6 未来扩展（V1.2 不包含）

* **并发与线程安全**：多线程撮合、锁与一致性保证。
* **网络与远程接口**：服务化、RPC/HTTP 接入。
* **数据库持久化**：替换 CSV，支持事务与恢复。
* **高级撮合与订单簿**：撮合优先级、部分成交与挂单簿。

## 附录 A：字段契约 Data Dictionary（V1.2）

目标：让你“不懂金融也能写对”。每个字段都写死：含义 / 单位 / 范围 / 来源 / 变化时机 / 失败策略
### A.1 基础类型与单位

**Money::cents_** (`long long`)  
含义：金额  
单位：分  
约束 / 不变量：可为负（用于中间计算），但**账户余额**必须 `>= 0`；价格必须 `> 0`（限价单）  
失败策略：违规抛 `InvalidArgumentException` / `InsufficientFundsException`

**qty** (`std::int64_t`)  
含义：数量（下单/持仓/成交）  
单位：股/份（抽象单位）  
约束 / 不变量：V1.2 不做空：**持仓 qty 必须 `>= 0`**；订单 qty 必须 `> 0`；成交 qty 必须 `> 0`  
失败策略：订单参数：`InvalidArgumentException`；持仓不足：`TradeSimException(ErrorCode::InsufficientPosition, ...)`

---
### A.2 标识符与枚举

**AccountId** (`std::string`)  
含义：账户标识  
约束 / 不变量：非空  
失败策略：空：`InvalidArgumentException`；不存在：`NotFoundException`

**Symbol** (`std::string`)  
含义：标的代码（比如 `AAPL`）  
约束 / 不变量：非空；字段内不含逗号（CSV 规则）  
失败策略：空：`InvalidArgumentException`

**OrderId** (`uint64_t`)  
含义：订单 ID  
约束 / 不变量：`> 0`；在 `OrderManager` 内唯一  
失败策略：重复：`TradeSimException(ErrorCode::Duplicate, ...)`

**TradeId** (`uint64_t`)  
含义：成交 ID  
约束 / 不变量：`> 0`；在 `HistoryManager` 内唯一  
失败策略：重复：`TradeSimException(ErrorCode::Duplicate, ...)`

**Side** (`enum class`)  
含义：买/卖  
约束 / 不变量：`Buy` / `Sell`  
失败策略：非法值（反序列化）：`ParseErrorException`

**OrderKind** (`enum class`)  
含义：市价/限价  
约束 / 不变量：`Market` / `Limit`  
失败策略：非法值（反序列化）：`ParseErrorException`

**OrderStatus** (`enum class`)  
含义：订单状态  
约束 / 不变量：`Pending/PartiallyFilled/Filled/Cancelled/Rejected`  
失败策略：非法状态转换：`TradeSimException(ErrorCode::InvalidState, ...)`

---
### A.3 struct / class 字段契约

#### Position（持仓）

**symbol** (`string`)  
含义：持仓标的  
约束 / 不变量：非空  
何时变化：只在创建/载入时确定  
失败策略：空：`InvalidArgumentException`；解析失败：`ParseErrorException`

**qty** (`int64_t`)  
含义：当前持仓数量  
约束 / 不变量：`>= 0`（V1.2 不做空）  
何时变化：结算成交时增减；载入文件  
失败策略：减到负：`TradeSimException(ErrorCode::InsufficientPosition, ...)`

#### Account（账户）

**id_** (`AccountId`)  
含义：用户 ID  
约束 / 不变量：非空  
何时变化：创建时确定  
失败策略：空：`InvalidArgumentException`

**balance_** (`Money`)  
含义：现金余额  
约束 / 不变量：`>= 0`（V1.2 不透支）  
何时变化：`deposit/withdraw`；成交结算  
失败策略：不足：`InsufficientFundsException`

**positions_** (`map(Symbol->qty)`)  
含义：持仓表  
约束 / 不变量：每个 `qty >= 0`  
何时变化：成交结算；载入文件  
失败策略：不足：`TradeSimException(ErrorCode::InsufficientPosition, ...)`

#### Order（订单层次）

**id_** (`OrderId`)  
含义：订单 ID  
约束 / 不变量：`> 0`  
何时变化：创建时确定  
失败策略：重复入库：`TradeSimException(ErrorCode::Duplicate, ...)`

**user_** (`AccountId`)  
含义：下单账户  
约束 / 不变量：非空  
何时变化：创建时确定  
失败策略：空：`InvalidArgumentException`

**symbol_** (`Symbol`)  
含义：标的  
约束 / 不变量：非空  
何时变化：创建时确定  
失败策略：空：`InvalidArgumentException`

**side_** (`Side`)  
含义：买/卖  
约束 / 不变量：必须是枚举合法值  
何时变化：创建时确定  
失败策略：非法：`InvalidArgumentException`

**qty_** (`int64_t`)  
含义：下单数量  
约束 / 不变量：`> 0`  
何时变化：创建时确定；（可选）撮合后剩余量变化  
失败策略：`qty <= 0`：`InvalidArgumentException`

**LimitOrder::limit_** (`Money`)  
含义：限价  
约束 / 不变量：`> 0`  
何时变化：创建时确定  
失败策略：`<= 0`：`InvalidArgumentException`

#### Trade（成交）

**tradeId** (`TradeId`)  
含义：成交 ID  
约束 / 不变量：`> 0`  
来源：`MatchingEngine` 生成  
失败策略：重复：`TradeSimException(ErrorCode::Duplicate, ...)`

**buyOrderId** (`OrderId`)  
含义：买单 ID  
约束 / 不变量：`> 0`  
来源：撮合结果  
失败策略：订单不存在：`InvalidState`（或 `NotFound`）

**sellOrderId** (`OrderId`)  
含义：卖单 ID  
约束 / 不变量：`> 0`  
来源：撮合结果  
失败策略：订单不存在：`InvalidState`（或 `NotFound`）

**symbol** (`Symbol`)  
含义：标的  
约束 / 不变量：非空  
来源：撮合结果  
失败策略：空：`InvalidState`

**qty** (`int64_t`)  
含义：成交数量  
约束 / 不变量：`> 0`  
来源：撮合结果  
失败策略：`<= 0`：`InvalidState`

**price** (`Money`)  
含义：成交价  
约束 / 不变量：`> 0`  
来源：撮合结果  
失败策略：`<= 0`：`InvalidState`

---
### 附录 B：落盘 CSV 契约（V1.2）

> path 语义：loadFromFile(path) / saveToFile(path) 的 path 是目录，文件名固定。
> 解析规则：无 header，允许空行，不支持转义，任何一行失败整文件失败抛 ParseErrorException。

| 文件              | 行格式                                                    | 字段约束                                           |
| --------------- | ------------------------------------------------------ | ---------------------------------------------- |
| `accounts.csv`  | `accountId,balanceCents`                               | `accountId` 非空；`balanceCents >= 0`             |
| `positions.csv` | `accountId,symbol,qty`                                 | `accountId` 非空；`symbol` 非空；`qty >= 0`          |
| `trades.csv`    | `tradeId,buyOrderId,sellOrderId,symbol,qty,priceCents` | `tradeId>0`；`qty>0`；`priceCents>0`；`symbol` 非空 |

### 附录 C：示例流程（Example Flows，带数字，能直接写测试）

### Flow 1：最小成交结算（无手续费）

初始：

- u1.balance = 1000.00（100000 分）
- u2.balance = 0.00
- u2.positions[AAPL] = 10

订单与成交：

- u1 买 AAPL 3 股
- 成交价 100.00（10000 分）
- 成交金额 notional = qty * price = 3 * 10000 = 30000 分

期望结果：

- u1.balance = 100000 - 30000 = 70000 分（700.00）
- u1.positions[AAPL] = +3
- u2.balance = 0 + 30000 = 30000 分（300.00）
- u2.positions[AAPL] = 10 - 3 = 7
- HistoryManager 中 u1/u2 都能查到同一条 Trade

Flow 2：资金不足（必须原子失败）

初始：

- u1.balance = 50.00（5000 分）
- 想买：AAPL 1 股，成交价 100.00（10000 分）

期望：

- applyTradeToAccounts 抛 InsufficientFundsException
- 账户状态不应被改一半：u1.balance 仍是 5000 分；持仓不变；历史不记录

Flow 3：持仓不足（必须原子失败）

初始：

- u2.positions[AAPL] = 0
- 卖出成交：AAPL 1 股

期望：

- 抛 TradeSimException(ErrorCode::InsufficientPosition, ...)
- u2.balance 不增加；u1.balance 不减少；历史不记录


---


