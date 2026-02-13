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
