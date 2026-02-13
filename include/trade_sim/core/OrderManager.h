#pragma once

#include "trade_sim/common/Types.h"
#include "trade_sim/order/Order.h"

#include <memory>
#include <unordered_map>

namespace trade_sim {

/**
 * OrderManager：订单生命周期管理
 * - 内部用 unique_ptr<Order> 持有（RAII）
 */
class OrderManager {
public:
    OrderId nextId() noexcept;

    void submit(std::unique_ptr<Order> order);
    Order& get(OrderId id);
    const Order& get(OrderId id) const;

    OrderStatus status(OrderId id) const;
    void cancel(OrderId id);

    // 裸指针入口（训练点），内部立刻接管为 unique_ptr
    void submitRaw(Order* rawOrder);

private:
    OrderId next_{1};
    std::unordered_map<OrderId, std::unique_ptr<Order>> orders_;
    std::unordered_map<OrderId, OrderStatus> status_;
};

} // namespace trade_sim
