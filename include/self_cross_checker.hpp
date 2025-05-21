#pragma once

#include "orderbook.hpp"

class SelfCrossChecker {
  public:
    SelfCrossChecker(OrderBook& orderBook);

    bool checkSelfCross(const Order* newOrder);

  private:
    OrderBook& orderBook_;
};
