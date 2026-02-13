#pragma once

#include "trade_sim/common/Exceptions.h"
#include "trade_sim/common/Types.h"
#include "trade_sim/model/Asset.h"

#include <unordered_map>
#include <utility>

namespace trade_sim {

/**
 * Account：账户与持仓。
 * - balance_：现金余额
 * - positions_：symbol -> qty
 */
class Account {
public:
    Account() = default;
    Account(AccountId id, Money initial)
        : id_(std::move(id)), balance_(initial) {}

    const AccountId& id() const noexcept { return id_; }
    Money balance() const noexcept { return balance_; }

    // 函数重载训练点：deposit 支持 Money / long long cents
    void deposit(const Money& amount) { balance_ += amount; }

    void deposit(long long cents) { deposit(Money(cents)); }

    void withdraw(const Money& amount) {
        if (balance_ < amount) {
            throw InsufficientFundsException("insufficient funds");
        }
        balance_ -= amount;
    }

    std::int64_t positionOf(const Symbol& sym) const {
        auto it = positions_.find(sym);
        return it == positions_.end() ? 0 : it->second;
    }

    void addPosition(const Symbol& sym, std::int64_t deltaQty) {
        const auto next = positionOf(sym) + deltaQty;
        if (next < 0) {
            throw TradeSimException(ErrorCode::InsufficientPosition, "insufficient position");
        }
        positions_[sym] = next;
    }

private:
    AccountId id_;
    Money balance_{0};
    std::unordered_map<Symbol, std::int64_t> positions_;
};

} // namespace trade_sim
