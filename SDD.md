# **金融交易模拟系统**详细设计说明书（SDD/LLD）

---

## 1. 详细设计说明书（SDD / LLD）

### 1.1 系统概述

金融交易模拟系统旨在模拟一个金融平台，支持用户的交易操作，包括股票、期货等资产的买卖。系统应支持账户管理、资产管理、订单管理、交易执行、历史查询等功能，并且具备多种类型的订单，如限价单、市场单等。每个用户可以通过账户进行资金管理、资产查询、交易提交等操作，系统需要高效、实时地处理大量的并发交易请求。

### 1.2 模块设计

#### 1.2.1 账户管理模块（AccountManager）

* **职责**：该模块负责用户账户的创建、信息查询、余额更新等操作。
* **数据结构**：

  * `Account` 类：表示每个用户的账户，包含账户余额、账户ID、账户类型等信息。
  * `AccountManager` 类：负责管理所有用户账户的创建、删除、余额更新、信息查询等操作。
* **接口设计**：

  * `createAccount(const std::string& userId, double initialBalance)`: 创建一个新的账户。
  * `getAccountInfo(const std::string& userId)`: 获取指定账户的信息。
  * `updateBalance(const std::string& userId, double amount)`: 更新指定账户的余额。
  * `deleteAccount(const std::string& userId)`: 删除指定的账户。

#### 1.2.2 资产管理模块（AssetManager）

* **职责**：该模块负责管理用户的资产（如股票、期货），并提供查看资产的功能。
* **数据结构**：

  * `Asset` 类：表示每个用户的资产，包含资产类型、资产数量、当前市值等。
  * `AssetManager` 类：负责管理所有用户的资产，支持资产的查询和更新。
* **接口设计**：

  * `getAssetInfo(const std::string& userId, const std::string& assetType)`: 获取指定用户指定资产的详细信息。
  * `updateAsset(const std::string& userId, const std::string& assetType, double amount)`: 更新用户的资产信息。

#### 1.2.3 订单管理模块（OrderManager）

* **职责**：该模块负责管理用户的交易订单，支持订单的创建、查询、取消等功能。
* **数据结构**：

  * `Order` 类：表示一个交易订单，包含订单ID、用户ID、资产类型、订单类型（限价单、市场单等）、交易数量、价格等。
  * `OrderManager` 类：负责管理所有用户的订单，支持订单的查询、提交、取消等操作。
* **接口设计**：

  * `createOrder(const Order& order)`: 提交一个新的交易订单。
  * `getOrderStatus(int orderId)`: 查询指定订单的状态。
  * `cancelOrder(int orderId)`: 取消指定的交易订单。

#### 1.2.4 交易执行模块（TradeExecutor）

* **职责**：该模块负责执行用户的交易订单，处理限价单、市场单等订单类型的执行。
* **数据结构**：

  * `Trade` 类：表示一次交易，包含交易ID、订单ID、交易资产、成交价格、成交数量等信息。
  * `TradeExecutor` 类：负责实际执行交易，更新账户余额和资产数量。
* **接口设计**：

  * `executeTrade(const Order& order)`: 执行一笔交易，并更新相应账户的余额和资产信息。
  * `getTradeHistory(const std::string& userId)`: 获取用户的交易历史记录。

#### 1.2.5 历史查询模块（HistoryManager）

* **职责**：该模块负责记录和查询用户的交易历史。
* **数据结构**：

  * `TradeHistory` 类：记录用户的交易历史，包括交易ID、订单ID、交易时间、交易金额、交易资产等信息。
  * `HistoryManager` 类：负责存储所有用户的交易历史。
* **接口设计**：

  * `getTradeHistory(const std::string& userId)`: 查询指定用户的交易历史记录。
  * `getTransactionDetails(int tradeId)`: 查询指定交易ID的详细信息。

### 1.3 关键类设计

#### 1.3.1 Account 类

* **成员变量**：

  * `std::string userId`: 用户ID。
  * `double balance`: 账户余额。
  * `std::unordered_map<std::string, Asset> assets`: 用户的资产信息，按资产类型（如股票、期货等）分类。

* **内存管理**：

  * 账户和资产使用标准容器（如 `std::unordered_map`）管理内存，确保内存自动管理。

#### 1.3.2 Order 类

* **成员变量**：

  * `int orderId`: 订单ID。
  * `std::string userId`: 用户ID。
  * `std::string assetType`: 交易资产类型。
  * `OrderType type`: 订单类型（限价单、市场单等）。
  * `double amount`: 订单数量。
  * `double price`: 交易价格（对于限价单有效）。

* **内存管理**：

  * 订单类不涉及复杂的内存管理，使用智能指针来管理订单对象。

#### 1.3.3 Trade 类

* **成员变量**：

  * `int tradeId`: 交易ID。
  * `int orderId`: 关联订单ID。
  * `std::string assetType`: 交易资产类型。
  * `double amount`: 交易数量。
  * `double price`: 成交价格。

* **内存管理**：

  * 交易记录存储在 `std::vector` 中，系统会自动管理内存。

### 1.4 内存管理

* **账户管理**：账户和资产使用标准容器（如 `std::unordered_map` 和 `std::vector`）管理，确保内存自动分配和释放。
* **订单管理**：订单对象由系统动态创建和销毁，使用智能指针管理内存，避免内存泄漏。
* **交易执行**：每次交易完成后，更新账户余额和资产信息。内存管理由智能指针和标准容器自动处理。

### 1.5 异常处理

* **资金不足**：在执行交易时，若账户余额不足，抛出 `InsufficientFundsException` 异常。
* **无效订单**：当订单参数无效（如价格为负数、数量为零等），抛出 `InvalidOrderException` 异常。
* **数据库操作失败**：在与数据库交互时发生错误（如存储失败），抛出 `DatabaseException` 异常。
* **超时和网络错误**：在并发环境下，若交易请求超时或发生网络错误，抛出 `NetworkException` 异常。

---

### 1.6 其他设计考量

* **线程安全**：订单提交、交易执行等操作需要线程安全。使用锁机制（如 `std::mutex`）保证并发情况下的正确性。
* **性能优化**：考虑到系统需要高效处理大量订单和交易，优化数据结构和算法，避免不必要的资源消耗。
* **持久化存储**：所有交易记录、账户信息等需要持久化存储，可以使用数据库（如 SQLite）或者文件系统进行存储。


