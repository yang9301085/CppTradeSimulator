#pragma once

#include "trade_sim/common/Exceptions.h"
#include "trade_sim/common/Types.h"

#include <string>
#include <utility>

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
    virtual Order* cloneRaw() const = 0; // 返回 new 对象，调用方负责 delete

protected:
    OrderId id_{0};
    AccountId user_;
    Symbol symbol_;
    Side side_{Side::Buy};
    std::int64_t qty_{0};
};

} // namespace trade_sim
