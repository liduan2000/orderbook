#ifndef RISKCONTROL_H
#define RISKCONTROL_H

#include "RateLimiter.h"
#include "SelfCrossChecker.h"

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

#endif  // RISKCONTROL_H
