#include "trade_sim/io/Storage.h"
#include "trade_sim/common/Exceptions.h"

#include <fstream>

namespace trade_sim {

std::vector<std::string> Storage::readAllLines(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw IOErrorException("cannot open file for read: " + path);

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) lines.push_back(line);
    return lines;
}

void Storage::writeAllLines(const std::string& path, const std::vector<std::string>& lines) {
    std::ofstream out(path, std::ios::trunc);
    if (!out) throw IOErrorException("cannot open file for write: " + path);

    for (const auto& s : lines) out << s << "\n";
}

} // namespace trade_sim
