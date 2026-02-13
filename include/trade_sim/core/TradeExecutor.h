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
