#pragma once

#include "trade_sim/order/Order.h"

#include <utility>

namespace trade_sim {

class MarketOrder final : public Order {
public:
    MarketOrder(OrderId id, AccountId user, Symbol sym, Side side, std::int64_t qty)
        : Order(id, std::move(user), std::move(sym), side, qty) {}

    OrderKind kind() const noexcept override { return OrderKind::Market; }
    Money limitPrice() const override { return Money(0); }
    Order* cloneRaw() const override { return new MarketOrder(*this); }
};

class LimitOrder final : public Order {
public:
    LimitOrder(OrderId id, AccountId user, Symbol sym, Side side, std::int64_t qty, Money limit)
        : Order(id, std::move(user), std::move(sym), side, qty), limit_(limit) {}

    OrderKind kind() const noexcept override { return OrderKind::Limit; }
    Money limitPrice() const override { return limit_; }
    Order* cloneRaw() const override { return new LimitOrder(*this); }

private:
    Money limit_{0};
};

} // namespace trade_sim
