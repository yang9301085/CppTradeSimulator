#pragma once

#include <cmath>
#include <cstdint>
#include <ostream>
#include <string>

namespace trade_sim {

/** 订单方向 */
enum class Side { Buy, Sell };

/** 订单状态 */
enum class OrderStatus { Pending, PartiallyFilled, Filled, Cancelled, Rejected };

/** 订单类型 */
enum class OrderKind { Market, Limit };

/** 简单的强类型 ID */
using AccountId = std::string;
using Symbol = std::string;
using OrderId = std::uint64_t;
using TradeId = std::uint64_t;

/**
 * Money：故意做个轻量封装，练运算符重载。
 * 这里用 long long 表示分，避免 double 的坑（现实系统也这么干）。
 */
class Money {
public:
    Money() = default;
    explicit Money(long long cents) : cents_(cents) {}

    static Money FromYuan(double yuan) {
        return Money(static_cast<long long>(std::llround(yuan * 100.0)));
    }

    long long cents() const noexcept { return cents_; }

    // 运算符重载（训练点）
    Money operator+(const Money& rhs) const noexcept { return Money(cents_ + rhs.cents_); }
    Money operator-(const Money& rhs) const noexcept { return Money(cents_ - rhs.cents_); }
    Money& operator+=(const Money& rhs) noexcept {
        cents_ += rhs.cents_;
        return *this;
    }
    Money& operator-=(const Money& rhs) noexcept {
        cents_ -= rhs.cents_;
        return *this;
    }

    bool operator<(const Money& rhs) const noexcept { return cents_ < rhs.cents_; }
    bool operator==(const Money& rhs) const noexcept { return cents_ == rhs.cents_; }

private:
    long long cents_{0};
};

inline std::ostream& operator<<(std::ostream& os, const Money& m) {
    os << m.cents();
    return os;
}

} // namespace trade_sim
