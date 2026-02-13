#include "trade_sim/common/Exceptions.h"
#include "trade_sim/core/AccountManager.h"
#include "trade_sim/core/HistoryManager.h"
#include "trade_sim/core/MatchingEngine.h"
#include "trade_sim/core/OrderManager.h"
#include "trade_sim/core/TradeExecutor.h"
#include "trade_sim/order/OrderFactory.h"

#include <iostream>

using namespace trade_sim;

int main() {
    AccountManager am;
    OrderManager om;
    MatchingEngine me;
    HistoryManager hm;
    TradeExecutor exec(am, om, me, hm);

    try {
        am.createAccount("u1", Money(1'000'00)); // 1000.00
        am.createAccount("u2", Money(1'000'00));

        // 演示：用 RAII 创建订单
        auto oid = om.nextId();
        auto order = OrderFactory::createLimitOrder(oid, "u1", "AAPL", Side::Buy, 10, Money(100'00));
        exec.submitAndProcess(std::move(order));

        std::cout << "OK\n";
    } catch (const TradeSimException& e) {
        std::cerr << "[TradeSimException] code=" << static_cast<int>(e.code()) << " msg=" << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "[std::exception] " << e.what() << "\n";
        return 1;
    }

    return 0;
}
