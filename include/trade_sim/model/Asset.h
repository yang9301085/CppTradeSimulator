#pragma once

#include <cstdint>
#include <string>

namespace trade_sim {

/** 持仓（struct 训练点） */
struct Position {
    std::string symbol;
    std::int64_t qty{0}; // 可为负？这里先不支持做空，TODO：扩展
};

} // namespace trade_sim
