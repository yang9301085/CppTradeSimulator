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
