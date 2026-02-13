#pragma once

#include <string>
#include <vector>

namespace trade_sim {

/**
 * Storage：集中放简单文件读写工具
 * 文件格式（硬约束）：
 * - accounts.csv：accountId,balanceCents
 * - positions.csv：accountId,symbol,qty
 * - trades.csv：tradeId,buyOrderId,sellOrderId,symbol,qty,priceCents
 * 规则：
 * - 无 header
 * - 允许空行
 * - 不支持转义（逗号即分隔符）
 * - 任一行解析失败：整文件失败并抛 ParseErrorException
 */
class Storage {
public:
    static std::vector<std::string> readAllLines(const std::string& path);
    static void writeAllLines(const std::string& path, const std::vector<std::string>& lines);
};

} // namespace trade_sim
