#include "trade_sim/common/Exceptions.h"
#include "trade_sim/core/AccountManager.h"
#include "trade_sim/order/OrderFactory.h"

#include <cassert>

using namespace trade_sim;

int main() {
    AccountManager am;

    am.createAccount("u1", Money(100));
    assert(am.exists("u1"));

    bool thrown = false;
    try {
        am.getAccount("not_exist");
    } catch (const NotFoundException&) {
        thrown = true;
    }
    assert(thrown);

    // 1) market qty=0 -> InvalidArgumentException
    thrown = false;
    try {
        auto o = OrderFactory::createMarketOrder(1, "u1", "AAPL", Side::Buy, 0);
        (void)o;
    } catch (const InvalidArgumentException&) {
        thrown = true;
    }
    assert(thrown);

    // 2) market symbol empty -> InvalidArgumentException
    thrown = false;
    try {
        auto o = OrderFactory::createMarketOrder(2, "u1", "", Side::Buy, 10);
        (void)o;
    } catch (const InvalidArgumentException&) {
        thrown = true;
    }
    assert(thrown);

    // 3) limit price=0 -> InvalidArgumentException
    thrown = false;
    try {
        auto o = OrderFactory::createLimitOrder(3, "u1", "AAPL", Side::Buy, 10, Money(0));
        (void)o;
    } catch (const InvalidArgumentException&) {
        thrown = true;
    }
    assert(thrown);

    // 4) valid limit order -> success + kind check
    auto ok = OrderFactory::createLimitOrder(4, "u1", "AAPL", Side::Buy, 10, Money(100));
    assert(ok);
    assert(ok->kind() == OrderKind::Limit);

    return 0;
}
