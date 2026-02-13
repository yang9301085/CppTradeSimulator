#include "trade_sim/core/AccountManager.h"
#include "trade_sim/io/Storage.h"

namespace trade_sim {

void AccountManager::createAccount(const AccountId& id, Money initial) {
    if (id.empty()) throw InvalidArgumentException("accountId is empty");
    if (accounts_.find(id) != accounts_.end()) throw TradeSimException(ErrorCode::Duplicate, "account already exists");
    accounts_.emplace(id, Account{id, initial});
}

Account& AccountManager::getAccount(const AccountId& id) {
    auto it = accounts_.find(id);
    if (it == accounts_.end()) throw NotFoundException("account not found: " + id);
    return it->second;
}

const Account& AccountManager::getAccount(const AccountId& id) const {
    auto it = accounts_.find(id);
    if (it == accounts_.end()) throw NotFoundException("account not found: " + id);
    return it->second;
}

bool AccountManager::exists(const AccountId& id) const noexcept {
    return accounts_.find(id) != accounts_.end();
}

void AccountManager::loadFromFile(const std::string& path) {
    // TODO: parse accounts.csv + positions.csv (path is a directory, fixed filenames)
    (void)path;
}

void AccountManager::saveToFile(const std::string& path) const {
    // TODO: write accounts.csv + positions.csv (path is a directory, fixed filenames)
    (void)path;
}

} // namespace trade_sim
