#pragma once

#include "trade_sim/model/Account.h"

#include <string>
#include <unordered_map>

namespace trade_sim {

/**
 * AccountManager：账户仓库（内存版 + 文件持久化接口）
 */
class AccountManager {
public:
    void createAccount(const AccountId& id, Money initial);
    Account& getAccount(const AccountId& id);
    const Account& getAccount(const AccountId& id) const;

    bool exists(const AccountId& id) const noexcept;

    // 文件读写（训练点）
    void loadFromFile(const std::string& path);
    void saveToFile(const std::string& path) const;

private:
    std::unordered_map<AccountId, Account> accounts_;
};

} // namespace trade_sim
