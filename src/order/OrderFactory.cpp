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
    delete p;
}

std::unique_ptr<Order> OrderFactory::createMarketOrder(OrderId id, AccountId user, Symbol sym, Side side, std::int64_t qty) {
    return std::unique_ptr<Order>(createMarketOrderRaw(id, std::move(user), std::move(sym), side, qty));
}

std::unique_ptr<Order> OrderFactory::createLimitOrder(OrderId id, AccountId user, Symbol sym, Side side, std::int64_t qty, Money limit) {
    return std::unique_ptr<Order>(createLimitOrderRaw(id, std::move(user), std::move(sym), side, qty, limit));
}

} // namespace trade_sim
