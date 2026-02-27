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
        const auto& buyOrder=orders_.get(t.buyOrderId);
        const auto& sellOrder=orders_.get(t.sellOrderId);
        history_.record(t, buyOrder.user(), sellOrder.user());
    }

    // TODO：更新订单状态 Pending -> Filled / PartiallyFilled
}

void TradeExecutor::applyTradeToAccounts(const Trade& t) {
    // TODO：这里是核心训练点：
    // - 买方：扣钱，加仓
    // - 卖方：加钱，减仓
    // - 异常安全：失败时不能把账户改一半
    if(t.qty<=0)throw TradeSimException(ErrorCode::InvalidState,"trade qty must be > 0");
    if(t.price.cents()<=0)throw TradeSimException(ErrorCode::InvalidState,"trade price must be > 0");
    if(t.symbol.empty())throw TradeSimException(ErrorCode::InvalidState,"trade symbol is empty");

    const auto& buyOrder=orders_.get(t.buyOrderId);
    const auto& sellOrder=orders_.get(t.sellOrderId);

    if (buyOrder.side() != Side::Buy) {
        throw TradeSimException(ErrorCode::InvalidState, "buyOrderId is not a buy order");
    }
    if (sellOrder.side() != Side::Sell) {
        throw TradeSimException(ErrorCode::InvalidState, "sellOrderId is not a sell order");
    }
    if (buyOrder.symbol() != t.symbol || sellOrder.symbol() != t.symbol) {
        throw TradeSimException(ErrorCode::InvalidState, "trade symbol mismatches orders");
    }

    auto& buyer = accounts_.getAccount(buyOrder.user());
    auto& seller = accounts_.getAccount(sellOrder.user());

    const Money notional(static_cast<long long>(t.qty) * t.price.cents());

    // 先预检查，避免“资金/持仓不足”导致半更新
    if (buyer.balance() < notional) {
        throw InsufficientFundsException("insufficient funds");
    }
    if (seller.positionOf(t.symbol) < t.qty) {
        throw TradeSimException(ErrorCode::InsufficientPosition, "insufficient position");
    }

    // 结算：买方加仓扣钱，卖方减仓加钱
    buyer.addPosition(t.symbol, t.qty);
    buyer.withdraw(notional);
    seller.deposit(notional);
    seller.addPosition(t.symbol, -t.qty);
}

} // namespace trade_sim
