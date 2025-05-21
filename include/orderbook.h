#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <map>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "order.h"

class OrderBook {
  public:
    void matchOrders(const std::string& symbol);

    void addOrder(const Order& order);

    bool cancelOrder(const std::string& orderId);

    bool modifyOrderQuantity(const std::string& orderId, size_t newQuantity);

    std::vector<Order> getOrdersForSymbol(const std::string& symbol, bool isBuy) const;

    std::pair<double, double> getBestPrices(const std::string& symbol) const;

    size_t getTotalOrderVolume(const std::string& symbol, bool isBuy) const;

  private:
    struct OrderContainer {
        std::multimap<double, Order, std::greater<double>> buyOrders; // buyOrders desc
        std::multimap<double, Order> sellOrders;                      // sellOrders asc
    };
    // read-write lock
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, OrderContainer> symbolOrderBooks_;
    std::unordered_map<std::string, Order> orderById_;
};
#endif // ORDERBOOK_H
