#include "trade_sim/core/HistoryManager.h"

namespace trade_sim {

void HistoryManager::record(const Trade& t, const AccountId& buyer, const AccountId& seller) {
    trades_[t.tradeId] = t;
    index_[buyer].push_back(t.tradeId);
    index_[seller].push_back(t.tradeId);
}

std::vector<Trade> HistoryManager::historyOf(const AccountId& user) const {
    std::vector<Trade> out;
    auto it = index_.find(user);
    if (it == index_.end()) return out;

    out.reserve(it->second.size());
    for (auto tid : it->second) {
        auto jt = trades_.find(tid);
        if (jt != trades_.end()) out.push_back(jt->second);
    }
    return out;
}

void HistoryManager::loadFromFile(const std::string& path) {
    // TODO: read trades.csv and rebuild index_ (path is a directory, fixed filename)
    (void)path;
}

void HistoryManager::saveToFile(const std::string& path) const {
    // TODO: write trades.csv (path is a directory, fixed filename)
    (void)path;
}

} // namespace trade_sim
