# 接口设计文档（API Spec）V1.2

> 版本：1.2

## 0. 口径约定

* 金额统一使用 `Money`（以分为单位，`long long` 存储）。
* 数量统一使用 `std::int64_t qty`。
* 模块划分与 `trade_sim` 目录一致：`common` / `model` / `order` / `core` / `io`。
* V1.2 为单进程内存版，持久化仅 CSV 文件读写；不包含并发、网络、数据库。

## 1. common 类型约定

### 1.1 Money

- **描述**：金额类型，内部以分为单位存储。
- **构造**：`Money(long long cents)`
- **转换**：`static Money FromYuan(double yuan)`
- **舍入**：`cents = llround(yuan * 100.0)`（四舍五入，远离 0）
- **参数约束**：`yuan` 必须为有限数值（非 NaN / 非 Inf）。
- **异常**：若 `yuan` 非法，抛 `InvalidArgumentException`。

### 1.2 基础类型与枚举

- **类型别名**：`AccountId` / `Symbol` / `OrderId` / `TradeId`
- **枚举**：`Side { Buy, Sell }`，`OrderKind { Market, Limit }`
- **订单状态**：`OrderStatus { Pending, PartiallyFilled, Filled, Cancelled, Rejected }`

### 1.3 异常

- **通用异常**：`InvalidArgumentException` / `NotFoundException` / `InsufficientFundsException`
- **IO/解析异常**：`IOErrorException` / `ParseErrorException`
- **统一错误码**：`TradeSimException(ErrorCode::Duplicate / InvalidState / InsufficientPosition)`
- **持仓不足**：`Account::addPosition` 抛 `TradeSimException(ErrorCode::InsufficientPosition, ...)`

## 2. model 模块

### 2.1 Position

- **字段**：`symbol`、`qty (std::int64_t)`

### 2.2 Account

- **字段**：`id (AccountId)`、`balance (Money)`、`positions (Symbol -> qty)`

## 3. order 模块

### 3.1 Order（抽象类）

- **字段**：`id` / `user` / `symbol` / `side` / `qty`
- **虚函数**：`OrderKind kind() const`，`Money limitPrice() const`
- **参数约束**：`qty > 0`，`symbol` 非空。

### 3.2 MarketOrder / LimitOrder

- **MarketOrder**：无价格，`limitPrice()` 返回 0。
- **LimitOrder**：持有限价 `Money limit`。

### 3.3 OrderFactory

#### 3.3.1 createMarketOrderRaw

- **签名**：`Order* createMarketOrderRaw(OrderId id, AccountId user, Symbol sym, Side side, std::int64_t qty)`
- **参数约束**：`sym` 非空，`qty > 0`。
- **异常安全**：强保证（失败无副作用）。
- **所有权**：返回裸指针，调用方拥有所有权，需调用 `destroyRaw` 或 `delete` 释放。

#### 3.3.2 createLimitOrderRaw

- **签名**：`Order* createLimitOrderRaw(OrderId id, AccountId user, Symbol sym, Side side, std::int64_t qty, Money limit)`
- **参数约束**：`sym` 非空，`qty > 0`，`limit.cents() > 0`。
- **异常安全**：强保证。
- **所有权**：返回裸指针，调用方拥有所有权。

#### 3.3.3 createMarketOrder

- **签名**：`std::unique_ptr<Order> createMarketOrder(...)`
- **参数约束**：同 `createMarketOrderRaw`。
- **异常安全**：强保证。
- **所有权**：返回 `unique_ptr`，调用方拥有所有权。

#### 3.3.4 createLimitOrder

- **签名**：`std::unique_ptr<Order> createLimitOrder(...)`
- **参数约束**：同 `createLimitOrderRaw`。
- **异常安全**：强保证。
- **所有权**：返回 `unique_ptr`，调用方拥有所有权。

#### 3.3.5 destroyRaw

- **签名**：`void destroyRaw(Order* p) noexcept`
- **参数约束**：允许 `nullptr`，无操作。
- **异常安全**：不抛异常。
- **所有权**：释放调用方持有的裸指针。

## 4. core 模块

### 4.1 AccountManager

#### 4.1.1 createAccount

- **签名**：`void createAccount(const AccountId& id, Money initial)`
- **参数约束**：`id` 非空；`initial.cents() >= 0`。
- **异常安全**：强保证。
- **所有权**：成功后账户归 `AccountManager` 持有。

#### 4.1.2 getAccount

- **签名**：`Account& getAccount(const AccountId& id)` / `const Account& getAccount(const AccountId& id) const`
- **参数约束**：`id` 必须存在。
- **异常安全**：强保证（无副作用）。
- **所有权**：返回内部引用，生命周期受 `AccountManager` 管理。

#### 4.1.3 exists

- **签名**：`bool exists(const AccountId& id) const noexcept`
- **参数约束**：无。
- **异常安全**：不抛异常。
- **所有权**：无。

#### 4.1.4 loadFromFile

- **签名**：`void loadFromFile(const std::string& path)`
- **参数约束**：`path` 视为目录，固定文件名为 `accounts.csv` / `positions.csv`，按 CSV 规则解析。
- **异常安全**：强保证（解析失败不修改状态）。
- **所有权**：无。

#### 4.1.5 saveToFile

- **签名**：`void saveToFile(const std::string& path) const`
- **参数约束**：`path` 视为目录，固定文件名为 `accounts.csv` / `positions.csv`。
- **异常安全**：基本保证（内存状态不变，文件可能部分写入）。
- **所有权**：无。

### 4.2 OrderManager

#### 4.2.1 nextId

- **签名**：`OrderId nextId() noexcept`
- **参数约束**：无。
- **异常安全**：不抛异常。
- **所有权**：无。

#### 4.2.2 submit

- **签名**：`void submit(std::unique_ptr<Order> order)`
- **参数约束**：`order` 非空；`order->id()` 不重复。
- **异常安全**：强保证（失败不入库）。
- **所有权**：成功后订单归 `OrderManager` 持有；失败时调用方仍拥有所有权。

#### 4.2.3 submitRaw

- **签名**：`void submitRaw(Order* rawOrder)`
- **参数约束**：`rawOrder` 非空。
- **异常安全**：强保证。
- **所有权**：函数开始即接管所有权；若提交失败，`rawOrder` 在栈展开时被销毁，不再回到调用方。

#### 4.2.4 get

- **签名**：`Order& get(OrderId id)` / `const Order& get(OrderId id) const`
- **参数约束**：`id` 必须存在。
- **异常安全**：强保证。
- **所有权**：返回内部引用，生命周期由 `OrderManager` 管理。

#### 4.2.5 status

- **签名**：`OrderStatus status(OrderId id) const`
- **参数约束**：`id` 必须存在。
- **异常安全**：强保证。
- **所有权**：无。

#### 4.2.6 cancel

- **签名**：`void cancel(OrderId id)`
- **参数约束**：`id` 必须存在且状态不可为 `Filled`。
- **异常安全**：强保证。
- **所有权**：无。

### 4.3 MatchingEngine

#### 4.3.1 match

- **签名**：`std::vector<Trade> match(const Order& incoming)`
- **参数约束**：`incoming.qty() > 0`，`incoming.symbol()` 非空。
- **异常安全**：强保证（失败不生成成交记录）。
- **所有权**：返回值为拷贝，调用方拥有。

### 4.4 TradeExecutor

#### 4.4.1 submitAndProcess

- **签名**：`void submitAndProcess(std::unique_ptr<Order> order)`
- **参数约束**：`order` 非空；订单参数合法。
- **异常安全**：基本保证（可能部分更新订单/账户/历史）。
- **所有权**：调用时即转移所有权；若 `OrderManager::submit` 成功，订单归 `OrderManager` 持有；若在提交前抛异常，订单在栈展开时被销毁。

### 4.5 HistoryManager

#### 4.5.1 record

- **签名**：`void record(const Trade& t, const AccountId& buyer, const AccountId& seller)`
- **参数约束**：`buyer` / `seller` 非空。
- **异常安全**：基本保证。
- **所有权**：`HistoryManager` 内部拷贝成交记录。

#### 4.5.2 historyOf

- **签名**：`std::vector<Trade> historyOf(const AccountId& user) const`
- **参数约束**：`user` 非空。
- **异常安全**：强保证。
- **所有权**：返回值为拷贝，调用方拥有。

#### 4.5.3 loadFromFile

- **签名**：`void loadFromFile(const std::string& path)`
- **参数约束**：`path` 视为目录，固定文件名为 `trades.csv`，按 CSV 规则解析。
- **异常安全**：强保证（解析失败不修改状态）。
- **所有权**：无。

#### 4.5.4 saveToFile

- **签名**：`void saveToFile(const std::string& path) const`
- **参数约束**：`path` 视为目录，固定文件名为 `trades.csv`。
- **异常安全**：基本保证（内存状态不变，文件可能部分写入）。
- **所有权**：无。

## 5. io 模块

### 5.1 Storage

#### 5.1.1 readAllLines

- **签名**：`std::vector<std::string> readAllLines(const std::string& path)`
- **参数约束**：`path` 为可读文件。
- **异常安全**：强保证（失败不产生输出）。
- **所有权**：返回值为拷贝，调用方拥有。

#### 5.1.2 writeAllLines

- **签名**：`void writeAllLines(const std::string& path, const std::vector<std::string>& lines)`
- **参数约束**：`path` 为可写文件。
- **异常安全**：基本保证（可能部分写入）。
- **所有权**：无。

### 5.2 CSV 解析规则

- 无 header
- 允许空行
- 不支持注释行
- 字段不做转义，字段内不可包含逗号
- 解析失败：整文件失败并抛 `ParseErrorException`
