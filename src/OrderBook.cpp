#include "OrderBook.h"

void OrderBook::matchOrders(const std::string& symbol) {
    std::unique_lock<std::shared_mutex> lock(mutex_);

    auto& orderContainer = symbolOrderBooks_[symbol];

    // order matches
    while (!orderContainer.buyOrders.empty() && !orderContainer.sellOrders.empty()) {
        auto bestBuyIt = orderContainer.buyOrders.begin();
        auto bestSellIt = orderContainer.sellOrders.begin();

        if (bestBuyIt->first >= bestSellIt->first) {
            if (bestBuyIt->second.qty > bestSellIt->second.qty) {
                orderContainer.sellOrders.erase(bestSellIt);
                bestBuyIt->second.qty -= bestSellIt->second.qty;
            } else if (bestBuyIt->second.qty < bestSellIt->second.qty) {
                orderContainer.buyOrders.erase(bestBuyIt);
                bestSellIt->second.qty -= bestBuyIt->second.qty;
            } else {
                orderContainer.buyOrders.erase(bestBuyIt);
                orderContainer.sellOrders.erase(bestSellIt);
            }

        } else {
            break;
        }
    }
}

void OrderBook::addOrder(const Order& order) {
    std::unique_lock<std::shared_mutex> lock(mutex_);

    auto& orderContainer = symbolOrderBooks_[order.symbol];
    if (order.isBuy) {
        orderContainer.buyOrders.insert({order.price, order});
    } else {
        orderContainer.sellOrders.insert({order.price, order});
    }
    orderById_[order.id] = order;
}

bool OrderBook::cancelOrder(const std::string& orderId) {
    std::unique_lock<std::shared_mutex> lock(mutex_);

    auto orderIt = orderById_.find(orderId);
    if (orderIt == orderById_.end()) {
        return false;
    }

    Order& order = orderIt->second;
    auto& orderContainer = symbolOrderBooks_[order.symbol];

    if (order.isBuy) {
        auto range = orderContainer.buyOrders.equal_range(order.price);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second.id == orderId) {
                orderContainer.buyOrders.erase(it);
                break;
            }
        }
    } else {
        auto range = orderContainer.sellOrders.equal_range(order.price);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second.id == orderId) {
                orderContainer.sellOrders.erase(it);
                break;
            }
        }
    }

    orderById_.erase(orderId);
    return true;
}

bool OrderBook::modifyOrderQuantity(const std::string& orderId, size_t newQuantity) {
    std::unique_lock<std::shared_mutex> lock(mutex_);

    auto orderIt = orderById_.find(orderId);
    if (orderIt == orderById_.end()) {
        return false;
    }

    Order& order = orderIt->second;
    order.qty = newQuantity;

    // if quantity is 0, cancel order
    if (newQuantity == 0) {
        return cancelOrder(orderId);
    }

    return true;
}

std::vector<Order> OrderBook::getOrdersForSymbol(const std::string& symbol, bool isBuy) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);

    std::vector<Order> orders;
    auto it = symbolOrderBooks_.find(symbol);
    if (it == symbolOrderBooks_.end()) {
        return orders;
    }

    const auto& orderContainer = it->second;
    if (isBuy) {
        const auto& targetOrders = orderContainer.buyOrders;
        for (const auto& orderPair : targetOrders) {
            orders.push_back(orderPair.second);
        }
    } else {
        const auto& targetOrders = orderContainer.sellOrders;
        for (const auto& orderPair : targetOrders) {
            orders.push_back(orderPair.second);
        }
    }

    return orders;
}

std::pair<double, double> OrderBook::getBestPrices(const std::string& symbol) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto it = symbolOrderBooks_.find(symbol);
    if (it == symbolOrderBooks_.end()) {
        return {0.0, 0.0};
    }
    const auto& orderContainer = symbolOrderBooks_.at(symbol);
    double bestBidPrice = orderContainer.buyOrders.empty() ? 0.0 : orderContainer.buyOrders.begin()->first;
    double bestAskPrice = orderContainer.sellOrders.empty() ? 0.0 : orderContainer.sellOrders.begin()->first;

    return {bestBidPrice, bestAskPrice};
}

size_t OrderBook::getTotalOrderVolume(const std::string& symbol, bool isBuy) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);

    auto it = symbolOrderBooks_.find(symbol);
    if (it == symbolOrderBooks_.end()) {
        return 0;
    }

    const auto& orderContainer = it->second;
    size_t totalVolume = 0;
    if (isBuy) {
        const auto& targetOrders = orderContainer.buyOrders;
        for (const auto& orderPair : targetOrders) {
            totalVolume += orderPair.second.qty;
        }
    } else {
        const auto& targetOrders = orderContainer.sellOrders;
        for (const auto& orderPair : targetOrders) {
            totalVolume += orderPair.second.qty;
        }
    }

    return totalVolume;
}
