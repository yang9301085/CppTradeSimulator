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

    static void destroyRaw(Order* p) noexcept;

    static std::unique_ptr<Order> createMarketOrder(OrderId id, AccountId user, Symbol sym, Side side, std::int64_t qty);
    static std::unique_ptr<Order> createLimitOrder(OrderId id, AccountId user, Symbol sym, Side side, std::int64_t qty, Money limit);
};

} // namespace trade_sim
