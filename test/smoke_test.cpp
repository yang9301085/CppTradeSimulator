#include "trade_sim/common/Exceptions.h"
#include "trade_sim/core/AccountManager.h"

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

    return 0;
}
