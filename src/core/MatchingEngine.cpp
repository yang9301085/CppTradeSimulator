#include "trade_sim/common/Exceptions.h"
#include "trade_sim/core/MatchingEngine.h"

namespace trade_sim {

std::vector<Trade> MatchingEngine::match(const Order& incoming) {
    // TODO：实现撮合逻辑
    // 建议 Iteration 1：先假装“总能成交”，成交价：
    // - Market：用固定价或传入行情
    // - Limit：用 limitPrice
    if(incoming.qty()<=0){
        throw InvalidArgumentException("incoming.qty must be > 0");
    }
    if(incoming.symbol().empty()){
        throw InvalidArgumentException("incoming.symbol is empty");
    }
    return {};
}

} // namespace trade_sim
