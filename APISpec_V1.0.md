
# 接口设计文档（API Spec）V1.0

> 版本：1.0

## 1.1 AccountManager 接口

### 1.1.1 `createAccount`

- **描述**：创建一个新的用户账户
- **签名**：`Account createAccount(const std::string& userId, double initialBalance)`
- **参数**：
    - `userId`: 用户ID（唯一标识符）
    - `initialBalance`: 初始余额（默认为0）
- **返回值**：新创建的 `Account` 对象
- **异常**：
    - `AccountAlreadyExistsException`: 账户已存在
    - `InvalidBalanceException`: 余额为负数

### 1.1.2 `getAccountInfo`

- **描述**：获取用户的账户信息
- **签名**：`Account getAccountInfo(const std::string& userId)`
- **参数**：`userId` - 用户ID
- **返回值**：`Account` 对象（含余额、资产信息等）
- **异常**：`AccountNotFoundException` - 账户不存在

### 1.1.3 `updateBalance`

- **描述**：更新账户余额
- **签名**：`void updateBalance(const std::string& userId, double amount)`
- **参数**：
    - `userId`: 用户ID
    - `amount`: 更新金额（正数或负数）
- **异常**：
    - `AccountNotFoundException` - 账户不存在
    - `InsufficientFundsException` - 余额不足

### 1.1.4 `deleteAccount`

- **描述**：删除用户账户
- **签名**：`void deleteAccount(const std::string& userId)`
- **参数**：`userId` - 用户ID
- **异常**：`AccountNotFoundException` - 账户不存在

## 1.2 AssetManager 接口

### 1.2.1 `getAssetInfo`

- **描述**：获取用户的资产详细信息
- **签名**：`Asset getAssetInfo(const std::string& userId, const std::string& assetType)`
- **参数**：
    - `userId`: 用户ID
    - `assetType`: 资产类型（如股票、期货等）
- **返回值**：`Asset` 对象（含数量、市值等）
- **异常**：
    - `AccountNotFoundException` - 账户不存在
    - `AssetNotFoundException` - 资产类型不存在

### 1.2.2 `updateAsset`

- **描述**：更新资产数量
- **签名**：`void updateAsset(const std::string& userId, const std::string& assetType, double amount)`
- **参数**：
    - `userId`: 用户ID
    - `assetType`: 资产类型
    - `amount`: 数量更新值（正数或负数）
- **异常**：
    - `AccountNotFoundException` - 账户不存在
    - `AssetNotFoundException` - 资产类型不存在

## 1.3 OrderManager 接口

### 1.3.1 `createOrder`

- **描述**：创建交易订单
- **签名**：`void createOrder(const Order& order)`
- **参数**：`order` - `Order` 对象（含用户ID、资产类型、订单类型、数量、价格等）
- **异常**：
    - `InvalidOrderException` - 订单无效（如数量为0、价格为负）
    - `InsufficientFundsException` - 资金不足

### 1.3.2 `getOrderStatus`

- **描述**：查询订单状态
- **签名**：`OrderStatus getOrderStatus(int orderId)`
- **参数**：`orderId` - 订单ID
- **返回值**：`OrderStatus` 枚举值（Pending、Executed、Cancelled）
- **异常**：`OrderNotFoundException` - 订单ID不存在

### 1.3.3 `cancelOrder`

- **描述**：取消订单
- **签名**：`void cancelOrder(int orderId)`
- **参数**：`orderId` - 订单ID
- **异常**：
    - `OrderNotFoundException` - 订单ID不存在
    - `OrderAlreadyExecutedException` - 订单已执行，无法取消

## 1.4 TradeExecutor 接口

### 1.4.1 `executeTrade`

- **描述**：执行交易
- **签名**：`void executeTrade(const Order& order)`
- **参数**：`order` - 交易订单对象
- **异常**：
    - `InvalidOrderException` - 订单无效
    - `InsufficientFundsException` - 账户资金不足

### 1.4.2 `getTradeHistory`

- **描述**：获取用户的交易历史
- **签名**：`std::vector<Trade> getTradeHistory(const std::string& userId)`
- **参数**：`userId` - 用户ID
- **返回值**：`std::vector<Trade>` - 所有交易记录
- **异常**：`AccountNotFoundException` - 账户不存在

## 1.5 HistoryManager 接口

### 1.5.1 `getTradeHistory`

- **描述**：获取用户的交易历史记录
- **签名**：`std::vector<TradeHistory> getTradeHistory(const std::string& userId)`
- **参数**：`userId` - 用户ID
- **返回值**：`std::vector<TradeHistory>` - 所有交易记录
- **异常**：`AccountNotFoundException` - 账户不存在

### 1.5.2 `getTransactionDetails`

- **描述**：查询交易的详细信息
- **签名**：`TradeHistory getTransactionDetails(int tradeId)`
- **参数**：`tradeId` - 交易ID
- **返回值**：`TradeHistory` 对象
- **异常**：`TradeNotFoundException` - 交易ID不存在

