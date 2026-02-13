#pragma once

#include "trade_sim/common/Types.h"
#include "trade_sim/order/Order.h"

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
