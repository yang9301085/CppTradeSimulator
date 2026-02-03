# 接口设计文档（API Spec）V1.1.1

> 版本：1.1.1

## 0. 口径约定

* 金额统一使用 `Money`（以分为单位，`long long` 存储）。
* 数量统一使用 `std::int64_t qty`。
* 模块划分与 `trade_sim` 目录一致：`common` / `model` / `order` / `core` / `io`。
* V1.1.1 为单进程内存版，持久化仅 CSV 文件读写；不包含并发、网络、数据库。

## 1. common 类型约定

### 1.1 Money

- **描述**：金额类型，内部以分为单位存储。
- **构造**：`Money(long long cents)`
- **转换**：`static Money FromYuan(double yuan)`
- **舍入**：`cents = llround(yuan * 100.0)`（四舍五入，远离 0）

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

### 3.2 MarketOrder / LimitOrder

- **MarketOrder**：无价格，`limitPrice()` 返回 0
- **LimitOrder**：携带限价 `Money limit`

### 3.3 OrderFactory

- **签名**：
  - `Order* createMarketOrderRaw(...)`
  - `Order* createLimitOrderRaw(...)`
  - `std::unique_ptr<Order> createMarketOrder(...)`
  - `std::unique_ptr<Order> createLimitOrder(...)`
  - `void destroyRaw(Order* p) noexcept`

## 4. core 模块

### 4.1 AccountManager

- **签名**：
  - `void createAccount(const AccountId& id, Money initial)`
  - `Account& getAccount(const AccountId& id)` / `const Account& getAccount(const AccountId& id) const`
  - `bool exists(const AccountId& id) const`
  - `void loadFromFile(const std::string& path)`
  - `void saveToFile(const std::string& path) const`
- **路径规则**：`path` 视为目录，固定文件名为 `accounts.csv` / `positions.csv`
- **异常**：`InvalidArgumentException` / `NotFoundException` / `TradeSimException(ErrorCode::Duplicate)` / `IOErrorException` / `ParseErrorException`

### 4.2 OrderManager

- **签名**：
  - `OrderId nextId() noexcept`
  - `void submit(std::unique_ptr<Order> order)`
  - `void submitRaw(Order* rawOrder)`
  - `Order& get(OrderId id)` / `const Order& get(OrderId id) const`
  - `OrderStatus status(OrderId id) const`
  - `void cancel(OrderId id)`
- **异常**：`InvalidArgumentException` / `NotFoundException` / `TradeSimException(ErrorCode::Duplicate / InvalidState)`

### 4.3 MatchingEngine

- **签名**：`std::vector<Trade> match(const Order& incoming)`

### 4.4 TradeExecutor

- **签名**：`void submitAndProcess(std::unique_ptr<Order> order)`
- **异常**：`InvalidArgumentException` / `InsufficientFundsException` / `TradeSimException(ErrorCode::InvalidState)`

### 4.5 HistoryManager

- **签名**：
  - `void record(const Trade& t, const AccountId& buyer, const AccountId& seller)`
  - `std::vector<Trade> historyOf(const AccountId& user) const`
  - `void loadFromFile(const std::string& path)`
  - `void saveToFile(const std::string& path) const`
- **路径规则**：`path` 视为目录，固定文件名为 `trades.csv`
- **异常**：`IOErrorException` / `ParseErrorException`

## 5. io 模块

### 5.1 Storage

- **签名**：
  - `std::vector<std::string> readAllLines(const std::string& path)`
  - `void writeAllLines(const std::string& path, const std::vector<std::string>& lines)`
- **异常**：`IOErrorException`


