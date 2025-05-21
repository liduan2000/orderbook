#pragma once

#include "rate_limiter.hpp"
#include "self_cross_checker.hpp"

class RiskControl {
  public:
    enum class RateLimiterType { FixedWindow, TokenBucket };

    RiskControl(OrderBook& orderBook, RateLimiterType rateLimiterType = RateLimiterType::FixedWindow,
                size_t maxRequestsPerInterval = 10, std::chrono::milliseconds interval = std::chrono::seconds(1));

    bool approveNewOrder(const Order* ord, std::string& rejectReason);

  private:
    SelfCrossChecker selfCrossChecker_;
    std::unique_ptr<RateLimiter> rateLimiter_;

    std::unique_ptr<RateLimiter> createRateLimiter(RateLimiterType rateLimiterType, size_t maxRequestsPerInterval,
                                                   std::chrono::milliseconds interval);
};
