#pragma once

#include <string>

struct Order {
    std::string id;
    std::string symbol;
    double price;
    double qty;
    bool isBuy; // true -> buy, false -> sell

    Order() = default;
    Order(const std::string& orderId, const std::string& orderSymbol, double orderPrice, double orderQty,
          bool orderIsBuy)
        : id(orderId), symbol(orderSymbol), price(orderPrice), qty(orderQty), isBuy(orderIsBuy) {}
};
