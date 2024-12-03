#include "SelfCrossChecker.h"

SelfCrossChecker::SelfCrossChecker(OrderBook& orderBook) : orderBook_(orderBook) {}

bool SelfCrossChecker::checkSelfCross(const Order* newOrder) {
    std::pair<double, double> bestPrices = orderBook_.getBestPrices(newOrder->symbol);
    if ((newOrder->isBuy && bestPrices.second != 0.0 && newOrder->price >= bestPrices.second) ||
        (!newOrder->isBuy && bestPrices.first != 0.0 && newOrder->price <= bestPrices.first)) {
        return false;
    }

    return true;
}
