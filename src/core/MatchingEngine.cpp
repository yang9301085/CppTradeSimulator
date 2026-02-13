#include "trade_sim/core/MatchingEngine.h"

namespace trade_sim {

std::vector<Trade> MatchingEngine::match(const Order& incoming) {
    // TODO：实现撮合逻辑
    // 建议 Iteration 1：先假装“总能成交”，成交价：
    // - Market：用固定价或传入行情
    // - Limit：用 limitPrice
    (void)incoming;
    return {};
}

} // namespace trade_sim
