#ifndef SELFCROSSCHECKER_H
#define SELFCROSSCHECKER_H

#include "orderbook.h"

class SelfCrossChecker {
  public:
    SelfCrossChecker(OrderBook& orderBook);

    bool checkSelfCross(const Order* newOrder);

  private:
    OrderBook& orderBook_;
};

#endif // SELFCROSSCHECKER_H