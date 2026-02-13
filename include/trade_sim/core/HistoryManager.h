#pragma once

#include "trade_sim/core/MatchingEngine.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace trade_sim {

/**
 * HistoryManager：保存交易历史
 * - userId -> vector<TradeId>
 * - trades_：tradeId -> Trade
 */
class HistoryManager {
public:
    void record(const Trade& t, const AccountId& buyer, const AccountId& seller);

    std::vector<Trade> historyOf(const AccountId& user) const;

    // 文件读写（训练点）
    void loadFromFile(const std::string& path);
    void saveToFile(const std::string& path) const;

private:
    std::unordered_map<TradeId, Trade> trades_;
    std::unordered_map<AccountId, std::vector<TradeId>> index_;
};

} // namespace trade_sim
