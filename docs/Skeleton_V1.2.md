# C++ 金融交易模拟系统代码骨架

> 版本：1.2

## V1.2 口径约定

- 金额统一使用 `Money`（以分为单位，`long long` 存储），数量统一使用 `std::int64_t qty`。
- 模块划分与目录一致：`common` / `model` / `order` / `core` / `io`，V1.2 不提供单独的 `AssetManager`。
- V1.2 为单进程内存版，持久化仅 CSV 文件读写；不包含并发、网络、数据库。

## 目录结构建议

```
CppTradeSimulator/
  CMakeLists.txt
  README.md
  include/
    trade_sim/
      common/
        Types.h
        Exceptions.h
      model/
        Asset.h
        Account.h
      order/
        Order.h
        Orders.h
        OrderFactory.h
      core/
        AccountManager.h
        OrderManager.h
        MatchingEngine.h
        TradeExecutor.h
        HistoryManager.h
      io/
        Storage.h
  src/
    core/
      AccountManager.cpp
      OrderManager.cpp
      MatchingEngine.cpp
      TradeExecutor.cpp
      HistoryManager.cpp
    io/
      Storage.cpp
    order/
      OrderFactory.cpp
    main.cpp
  test/
    smoke_test.cpp
```

---

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(CppTradeSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(trade_sim
  src/core/AccountManager.cpp
  src/core/OrderManager.cpp
  src/core/MatchingEngine.cpp
  src/core/TradeExecutor.cpp
  src/core/HistoryManager.cpp
  src/io/Storage.cpp
  src/order/OrderFactory.cpp
)

target_include_directories(trade_sim PUBLIC include)

add_executable(trade_sim_cli src/main.cpp)
target_link_libraries(trade_sim_cli PRIVATE trade_sim)

add_executable(smoke_test test/smoke_test.cpp)
target_link_libraries(smoke_test PRIVATE trade_sim)
```

---

## include/trade_sim/common/Types.h

```cpp
#pragma once
#include <cstdint>
#include <string>
#include <ostream>

namespace trade_sim {

/** 订单方向 */
enum class Side { Buy, Sell };

/** 订单状态 */
enum class OrderStatus { Pending, PartiallyFilled, Filled, Cancelled, Rejected };

/** 订单类型 */
enum class OrderKind { Market, Limit };

/** 简单的强类型 ID */
using AccountId = std::string;
using Symbol = std::string;
using OrderId = std::uint64_t;
using TradeId = std::uint64_t;

/**
 * Money：故意做个轻量封装，练运算符重载。
 * 这里用 long long 表示分，避免 double 的坑（现实系统也这么干）。
 */
class Money {
public:
    Money() = default;
    explicit Money(long long cents) : cents_(cents) {}

    static Money FromYuan(double yuan); // cents = llround(yuan * 100.0)
    long long cents() const noexcept { return cents_; }

    // 运算符重载（训练点）
    Money operator+(const Money& rhs) const noexcept { return Money(cents_ + rhs.cents_); }
    Money operator-(const Money& rhs) const noexcept { return Money(cents_ - rhs.cents_); }
    Money& operator+=(const Money& rhs) noexcept { cents_ += rhs.cents_; return *this; }
    Money& operator-=(const Money& rhs) noexcept { cents_ -= rhs.cents_; return *this; }

    bool operator<(const Money& rhs) const noexcept { return cents_ < rhs.cents_; }
    bool operator==(const Money& rhs) const noexcept { return cents_ == rhs.cents_; }

private:
    long long cents_{0};
};

std::ostream& operator<<(std::ostream& os, const Money& m);

} // namespace trade_sim
```

---

## include/trade_sim/common/Exceptions.h

```cpp
#pragma once
#include <stdexcept>
#include <string>

namespace trade_sim {

/** 错误码（训练：异常类型选择 vs 错误码） */
enum class ErrorCode {
    InvalidArgument,
    NotFound,
    Duplicate,
    InsufficientFunds,
    InsufficientPosition,
    IOError,
    ParseError,
    InvalidState
};

class TradeSimException : public std::runtime_error {
public:
    TradeSimException(ErrorCode code, const std::string& msg)
        : std::runtime_error(msg), code_(code) {}
    ErrorCode code() const noexcept { return code_; }

private:
    ErrorCode code_;
};

class NotFoundException : public TradeSimException {
public:
    explicit NotFoundException(const std::string& msg)
        : TradeSimException(ErrorCode::NotFound, msg) {}
};

class InvalidArgumentException : public TradeSimException {
public:
    explicit InvalidArgumentException(const std::string& msg)
        : TradeSimException(ErrorCode::InvalidArgument, msg) {}
};

class InsufficientFundsException : public TradeSimException {
public:
    explicit InsufficientFundsException(const std::string& msg)
        : TradeSimException(ErrorCode::InsufficientFunds, msg) {}
};

class IOErrorException : public TradeSimException {
public:
    explicit IOErrorException(const std::string& msg)
        : TradeSimException(ErrorCode::IOError, msg) {}
};

class ParseErrorException : public TradeSimException {
public:
    explicit ParseErrorException(const std::string& msg)
        : TradeSimException(ErrorCode::ParseError, msg) {}
};

} // namespace trade_sim
```

---

## include/trade_sim/model/Asset.h

```cpp
#pragma once
#include <cstdint>
#include <string>

namespace trade_sim {

/** 持仓（struct 训练点） */
struct Position {
    std::string symbol;
    std::int64_t qty{0};   // 可为负？这里先不支持做空，TODO：扩展
};

} // namespace trade_sim
```

---

## include/trade_sim/model/Account.h

```cpp
#pragma once
#include "trade_sim/common/Types.h"
#include "trade_sim/common/Exceptions.h"
#include "trade_sim/model/Asset.h"
#include <unordered_map>

namespace trade_sim {

/**
 * Account：账户与持仓。
 * - balance_：现金余额
 * - positions_：symbol -> qty
 */
class Account {
public:
    Account() = default;
    Account(AccountId id, Money initial)
        : id_(std::move(id)), balance_(initial) {}

    const AccountId& id() const noexcept { return id_; }
    Money balance() const noexcept { return balance_; }

    // 函数重载训练点：deposit 支持 Money / long long cents
    void deposit(const Money& amount);
    void deposit(long long cents);

    void withdraw(const Money& amount); // 不足抛 InsufficientFundsException

    std::int64_t positionOf(const Symbol& sym) const;
    void addPosition(const Symbol& sym, std::int64_t deltaQty); // throws TradeSimException(ErrorCode::InsufficientPosition, ...)

private:
    AccountId id_;
    Money balance_{0};
    std::unordered_map<Symbol, std::int64_t> positions_;
};

} // namespace trade_sim
```

---

## include/trade_sim/order/Order.h（抽象类 + 多态）

```cpp
#pragma once
#include "trade_sim/common/Types.h"
#include "trade_sim/common/Exceptions.h"
#include <string>

namespace trade_sim {

/**
 * Order：抽象订单基类（接口 + 多态训练点）
 * 约定：
 * - qty > 0
 * - symbol 非空
 */
class Order {
public:
    Order(OrderId id, AccountId user, Symbol sym, Side side, std::int64_t qty)
        : id_(id), user_(std::move(user)), symbol_(std::move(sym)), side_(side), qty_(qty) {}

    virtual ~Order() = default;

    OrderId id() const noexcept { return id_; }
    const AccountId& user() const noexcept { return user_; }
    const Symbol& symbol() const noexcept { return symbol_; }
    Side side() const noexcept { return side_; }
    std::int64_t qty() const noexcept { return qty_; }

    virtual OrderKind kind() const noexcept = 0;

    /** Limit 才有意义，Market 可以返回 0 或抛异常（你决定） */
    virtual Money limitPrice() const = 0;

    /** clone：演示多态拷贝（可选训练点） */
    virtual Order* cloneRaw() const = 0; // 注意：返回 new 出来的对象，调用方负责 delete（训练点）

protected:
    OrderId id_{0};
    AccountId user_;
    Symbol symbol_;
    Side side_{Side::Buy};
    std::int64_t qty_{0};
};

} // namespace trade_sim
```

---

## include/trade_sim/order/Orders.h（继承实现）

```cpp
#pragma once
#include "trade_sim/order/Order.h"

namespace trade_sim {

class MarketOrder final : public Order {
public:
    MarketOrder(OrderId id, AccountId user, Symbol sym, Side side, std::int64_t qty)
        : Order(id, std::move(user), std::move(sym), side, qty) {}

    OrderKind kind() const noexcept override { return OrderKind::Market; }
    Money limitPrice() const override { return Money(0); } // Market：无价格
    Order* cloneRaw() const override { return new MarketOrder(*this); } // new（训练点）
};

class LimitOrder final : public Order {
public:
    LimitOrder(OrderId id, AccountId user, Symbol sym, Side side, std::int64_t qty, Money limit)
        : Order(id, std::move(user), std::move(sym), side, qty), limit_(limit) {}

    OrderKind kind() const noexcept override { return OrderKind::Limit; }
    Money limitPrice() const override { return limit_; }
    Order* cloneRaw() const override { return new LimitOrder(*this); } // new（训练点）

private:
    Money limit_{0};
};

} // namespace trade_sim
```

---

## include/trade_sim/order/OrderFactory.h（显式 new/delete + RAII 对比）

```cpp
#pragma once
#include "trade_sim/order/Orders.h"
#include <memory>

namespace trade_sim {

/**
 * OrderFactory：
 * - createXXXRaw：返回裸指针，调用方必须 delete（训练点）
 * - createXXX：返回 unique_ptr，RAII（正确姿势）
 */
class OrderFactory {
public:
    static Order* createMarketOrderRaw(OrderId id, AccountId user, Symbol sym, Side side, std::int64_t qty);
    static Order* createLimitOrderRaw(OrderId id, AccountId user, Symbol sym, Side side, std::int64_t qty, Money limit);

    static void destroyRaw(Order* p) noexcept; // delete（训练点）

    static std::unique_ptr<Order> createMarketOrder(OrderId id, AccountId user, Symbol sym, Side side, std::int64_t qty);
    static std::unique_ptr<Order> createLimitOrder(OrderId id, AccountId user, Symbol sym, Side side, std::int64_t qty, Money limit);
};

} // namespace trade_sim
```

---

## include/trade_sim/core/AccountManager.h

```cpp
#pragma once
#include "trade_sim/model/Account.h"
#include <unordered_map>

namespace trade_sim {

/**
 * AccountManager：账户仓库（内存版 + 文件持久化接口）
 */
class AccountManager {
public:
    void createAccount(const AccountId& id, Money initial);
    Account& getAccount(const AccountId& id);
    const Account& getAccount(const AccountId& id) const;

    bool exists(const AccountId& id) const noexcept;

    // 文件读写（训练点）
    void loadFromFile(const std::string& path);
    void saveToFile(const std::string& path) const;

private:
    std::unordered_map<AccountId, Account> accounts_;
};

} // namespace trade_sim
```

---

## include/trade_sim/core/OrderManager.h

```cpp
#pragma once
#include "trade_sim/order/Order.h"
#include "trade_sim/common/Types.h"
#include <unordered_map>
#include <memory>

namespace trade_sim {

/**
 * OrderManager：订单生命周期管理
 * - 内部用 unique_ptr<Order> 持有（RAII）
 */
class OrderManager {
public:
    OrderId nextId() noexcept;

    void submit(std::unique_ptr<Order> order); // 转移所有权
    Order& get(OrderId id);
    const Order& get(OrderId id) const;

    OrderStatus status(OrderId id) const;
    void cancel(OrderId id);

    // 示例：裸指针入口（训练点），内部立刻接管为 unique_ptr
    void submitRaw(Order* rawOrder);

private:
    OrderId next_{1};
    std::unordered_map<OrderId, std::unique_ptr<Order>> orders_;
    std::unordered_map<OrderId, OrderStatus> status_;
};

} // namespace trade_sim
```

---

## include/trade_sim/core/MatchingEngine.h

```cpp
#pragma once
#include "trade_sim/order/Order.h"
#include "trade_sim/common/Types.h"
#include <vector>

namespace trade_sim {

/** 成交记录（struct 训练点） */
struct Trade {
    TradeId tradeId{0};
    OrderId buyOrderId{0};
    OrderId sellOrderId{0};
    Symbol symbol;
    std::int64_t qty{0};
    Money price{0};
};

/**
 * MatchingEngine：撮合引擎（简化版）
 * - 你可以先做“立即成交”假设
 * - 后续再扩展 order book
 */
class MatchingEngine {
public:
    std::vector<Trade> match(const Order& incoming);

private:
    TradeId nextTradeId_{1};
};

} // namespace trade_sim
```

---

## include/trade_sim/core/HistoryManager.h

```cpp
#pragma once
#include "trade_sim/core/MatchingEngine.h"
#include <vector>
#include <unordered_map>

namespace trade_sim {

/**
 * HistoryManager：保存交易历史
 * - userId -> vector<TradeId>
 * - trades_：tradeId -> Trade
 */
class HistoryManager {
public:
    void record(const Trade& t, const AccountId& buyer, const AccountId& seller);

    std::vector<Trade> historyOf(const AccountId& user) const;

    // 文件读写（训练点）
    void loadFromFile(const std::string& path);
    void saveToFile(const std::string& path) const;

private:
    std::unordered_map<TradeId, Trade> trades_;
    std::unordered_map<AccountId, std::vector<TradeId>> index_;
};

} // namespace trade_sim
```

---

## include/trade_sim/core/TradeExecutor.h

```cpp
#pragma once
#include "trade_sim/core/AccountManager.h"
#include "trade_sim/core/HistoryManager.h"
#include "trade_sim/core/MatchingEngine.h"
#include "trade_sim/core/OrderManager.h"

namespace trade_sim {

/**
 * TradeExecutor：业务编排层
 * 典型流程：
 * submit order -> matching -> apply account changes -> record history
 */
class TradeExecutor {
public:
    TradeExecutor(AccountManager& am, OrderManager& om, MatchingEngine& me, HistoryManager& hm)
        : accounts_(am), orders_(om), engine_(me), history_(hm) {}

    void submitAndProcess(std::unique_ptr<Order> order);

private:
    AccountManager& accounts_;
    OrderManager& orders_;
    MatchingEngine& engine_;
    HistoryManager& history_;

    void applyTradeToAccounts(const Trade& t); // TODO：扣钱/加仓/异常处理
};

} // namespace trade_sim
```

---

## include/trade_sim/io/Storage.h（文件格式集中）

```cpp
#pragma once
#include <string>
#include <vector>

namespace trade_sim {

/**
 * Storage：集中放简单文件读写工具
 * 文件格式建议（先别搞 JSON，练解析就用 CSV）：
 * - accounts.csv：accountId,balanceCents
 * - positions.csv：accountId,symbol,qty
 * - trades.csv：tradeId,buyOrderId,sellOrderId,symbol,qty,priceCents
 */
class Storage {
public:
    static std::vector<std::string> readAllLines(const std::string& path);
    static void writeAllLines(const std::string& path, const std::vector<std::string>& lines);
};

} // namespace trade_sim
```

---

# src 实现（全部 TODO，但保证能编译）

## src/order/OrderFactory.cpp

```cpp
#include "trade_sim/order/OrderFactory.h"
#include "trade_sim/common/Exceptions.h"

namespace trade_sim {

Order* OrderFactory::createMarketOrderRaw(OrderId id, AccountId user, Symbol sym, Side side, std::int64_t qty) {
    // TODO：参数校验（sym 非空，qty > 0）
    return new MarketOrder(id, std::move(user), std::move(sym), side, qty);
}

Order* OrderFactory::createLimitOrderRaw(OrderId id, AccountId user, Symbol sym, Side side, std::int64_t qty, Money limit) {
    // TODO：参数校验（limit > 0）
    return new LimitOrder(id, std::move(user), std::move(sym), side, qty, limit);
}

void OrderFactory::destroyRaw(Order* p) noexcept {
    delete p; // delete（训练点）
}

std::unique_ptr<Order> OrderFactory::createMarketOrder(OrderId id, AccountId user, Symbol sym, Side side, std::int64_t qty) {
    return std::unique_ptr<Order>(createMarketOrderRaw(id, std::move(user), std::move(sym), side, qty));
}

std::unique_ptr<Order> OrderFactory::createLimitOrder(OrderId id, AccountId user, Symbol sym, Side side, std::int64_t qty, Money limit) {
    return std::unique_ptr<Order>(createLimitOrderRaw(id, std::move(user), std::move(sym), side, qty, limit));
}

} // namespace trade_sim
```

---

## src/core/AccountManager.cpp

```cpp
#include "trade_sim/core/AccountManager.h"
#include "trade_sim/io/Storage.h"
#include <sstream>

namespace trade_sim {

void AccountManager::createAccount(const AccountId& id, Money initial) {
    if (id.empty()) throw InvalidArgumentException("accountId is empty");
    if (accounts_.find(id) != accounts_.end()) throw TradeSimException(ErrorCode::Duplicate, "account already exists");
    accounts_.emplace(id, Account{id, initial});
}

Account& AccountManager::getAccount(const AccountId& id) {
    auto it = accounts_.find(id);
    if (it == accounts_.end()) throw NotFoundException("account not found: " + id);
    return it->second;
}

const Account& AccountManager::getAccount(const AccountId& id) const {
    auto it = accounts_.find(id);
    if (it == accounts_.end()) throw NotFoundException("account not found: " + id);
    return it->second;
}

bool AccountManager::exists(const AccountId& id) const noexcept {
    return accounts_.find(id) != accounts_.end();
}

void AccountManager::loadFromFile(const std::string& path) {
    // TODO: parse accounts.csv + positions.csv (path is a directory, fixed filenames)
    (void)path;
}

void AccountManager::saveToFile(const std::string& path) const {
    // TODO: write accounts.csv + positions.csv (path is a directory, fixed filenames)
    (void)path;
}

} // namespace trade_sim
```

---

## src/core/OrderManager.cpp

```cpp
#include "trade_sim/core/OrderManager.h"
#include "trade_sim/common/Exceptions.h"

namespace trade_sim {

OrderId OrderManager::nextId() noexcept {
    return next_++;
}

void OrderManager::submit(std::unique_ptr<Order> order) {
    if (!order) throw InvalidArgumentException("submit: order is null");
    const auto id = order->id();
    if (orders_.find(id) != orders_.end()) throw TradeSimException(ErrorCode::Duplicate, "orderId duplicate");
    status_[id] = OrderStatus::Pending;
    orders_[id] = std::move(order);
}

void OrderManager::submitRaw(Order* rawOrder) {
    if (!rawOrder) throw InvalidArgumentException("submitRaw: null");
    // 立刻接管所有权，避免泄漏
    submit(std::unique_ptr<Order>(rawOrder));
}

Order& OrderManager::get(OrderId id) {
    auto it = orders_.find(id);
    if (it == orders_.end()) throw NotFoundException("order not found");
    return *(it->second);
}

const Order& OrderManager::get(OrderId id) const {
    auto it = orders_.find(id);
    if (it == orders_.end()) throw NotFoundException("order not found");
    return *(it->second);
}

OrderStatus OrderManager::status(OrderId id) const {
    auto it = status_.find(id);
    if (it == status_.end()) throw NotFoundException("order status not found");
    return it->second;
}

void OrderManager::cancel(OrderId id) {
    auto it = status_.find(id);
    if (it == status_.end()) throw NotFoundException("order not found");
    if (it->second == OrderStatus::Filled) throw TradeSimException(ErrorCode::InvalidState, "filled order cannot cancel");
    it->second = OrderStatus::Cancelled;
    // TODO：撮合队列里也要移除（后续扩展 OrderBook 时做）
}

} // namespace trade_sim
```

---

## src/core/MatchingEngine.cpp

```cpp
#include "trade_sim/core/MatchingEngine.h"

namespace trade_sim {

std::vector<Trade> MatchingEngine::match(const Order& incoming) {
    // TODO：实现撮合逻辑
    // 建议 Iteration 1：先假装“总能成交”，成交价：
    // - Market：用固定价或传入行情
    // - Limit：用 limitPrice
    (void)incoming;
    return {};
}

} // namespace trade_sim
```

---

## src/core/HistoryManager.cpp

```cpp
#include "trade_sim/core/HistoryManager.h"

namespace trade_sim {

void HistoryManager::record(const Trade& t, const AccountId& buyer, const AccountId& seller) {
    trades_[t.tradeId] = t;
    index_[buyer].push_back(t.tradeId);
    index_[seller].push_back(t.tradeId);
}

std::vector<Trade> HistoryManager::historyOf(const AccountId& user) const {
    std::vector<Trade> out;
    auto it = index_.find(user);
    if (it == index_.end()) return out;

    out.reserve(it->second.size());
    for (auto tid : it->second) {
        auto jt = trades_.find(tid);
        if (jt != trades_.end()) out.push_back(jt->second);
    }
    return out;
}

void HistoryManager::loadFromFile(const std::string& path) {
    // TODO: read trades.csv and rebuild index_ (path is a directory, fixed filename)
    (void)path;
}

void HistoryManager::saveToFile(const std::string& path) const {
    // TODO: write trades.csv (path is a directory, fixed filename)
    (void)path;
}

} // namespace trade_sim
```

---

## src/core/TradeExecutor.cpp

```cpp
#include "trade_sim/core/TradeExecutor.h"
#include "trade_sim/common/Exceptions.h"

namespace trade_sim {

void TradeExecutor::submitAndProcess(std::unique_ptr<Order> order) {
    if (!order) throw InvalidArgumentException("submitAndProcess: null order");

    // 1) 入库（OrderManager 持有所有权）
    const auto oid = order->id();
    orders_.submit(std::move(order));

    // 2) 撮合
    const Order& ref = orders_.get(oid);
    auto trades = engine_.match(ref);

    // 3) 应用成交 + 记录历史
    for (const auto& t : trades) {
        applyTradeToAccounts(t);
        // TODO：buyer/seller 如何确定：根据订单方向映射 orderId -> userId
        // 暂时先不写死
    }

    // TODO：更新订单状态 Pending -> Filled / PartiallyFilled
}

void TradeExecutor::applyTradeToAccounts(const Trade& t) {
    // TODO：这里是核心训练点：
    // - 买方：扣钱，加仓
    // - 卖方：加钱，减仓
    // - 异常安全：失败时不能把账户改一半
    (void)t;
}

} // namespace trade_sim
```

---

## src/io/Storage.cpp

```cpp
#include "trade_sim/io/Storage.h"
#include "trade_sim/common/Exceptions.h"
#include <fstream>

namespace trade_sim {

std::vector<std::string> Storage::readAllLines(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw IOErrorException("cannot open file for read: " + path);

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) lines.push_back(line);
    return lines;
}

void Storage::writeAllLines(const std::string& path, const std::vector<std::string>& lines) {
    std::ofstream out(path, std::ios::trunc);
    if (!out) throw IOErrorException("cannot open file for write: " + path);

    for (const auto& s : lines) out << s << "\n";
}

} // namespace trade_sim
```

---

## src/main.cpp（仅演示调用，不写业务）

```cpp
#include "trade_sim/core/AccountManager.h"
#include "trade_sim/core/OrderManager.h"
#include "trade_sim/core/MatchingEngine.h"
#include "trade_sim/core/TradeExecutor.h"
#include "trade_sim/core/HistoryManager.h"
#include "trade_sim/order/OrderFactory.h"
#include <iostream>

using namespace trade_sim;

int main() {
    AccountManager am;
    OrderManager om;
    MatchingEngine me;
    HistoryManager hm;
    TradeExecutor exec(am, om, me, hm);

    try {
        am.createAccount("u1", Money(1'000'00)); // 1000.00
        am.createAccount("u2", Money(1'000'00));

        // 演示：用 RAII 创建订单
        auto oid = om.nextId();
        auto order = OrderFactory::createLimitOrder(oid, "u1", "AAPL", Side::Buy, 10, Money(100'00));
        exec.submitAndProcess(std::move(order));

        std::cout << "OK\n";
    } catch (const TradeSimException& e) {
        std::cerr << "[TradeSimException] code=" << static_cast<int>(e.code()) << " msg=" << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "[std::exception] " << e.what() << "\n";
        return 1;
    }

    return 0;
}
```

---

## test/smoke_test.cpp（不需要框架，先跑通）

```cpp
#include "trade_sim/core/AccountManager.h"
#include "trade_sim/common/Exceptions.h"
#include <cassert>

using namespace trade_sim;

int main() {
    AccountManager am;

    am.createAccount("u1", Money(100));
    assert(am.exists("u1"));

    bool thrown = false;
    try {
        am.getAccount("not_exist");
    } catch (const NotFoundException&) {
        thrown = true;
    }
    assert(thrown);

    return 0;
}
```

---

