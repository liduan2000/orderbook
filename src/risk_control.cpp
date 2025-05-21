#include <memory>
#include <stdexcept>

#include "fixed_window_rate_limiter.h"
#include "risk_control.h"
#include "token_bucket_rate_limiter.h"

RiskControl::RiskControl(OrderBook& orderBook, RateLimiterType rateLimiterType, size_t maxRequests,
                         std::chrono::milliseconds interval)
    : selfCrossChecker_(orderBook), rateLimiter_(createRateLimiter(rateLimiterType, maxRequests, interval)) {}

bool RiskControl::approveNewOrder(const Order* ord, std::string& rejectReason) {
    if (!ord) {
        rejectReason = "Invalid order pointer";
        return false;
    }

    if (!selfCrossChecker_.checkSelfCross(ord)) {
        rejectReason = "Self-cross detected";
        return false;
    }

    if (!rateLimiter_->checkRequestRate()) {
        rejectReason = "Request rate exceeded";
        return false;
    }
    rejectReason.clear();

    return true;
}

std::unique_ptr<RateLimiter> RiskControl::createRateLimiter(RateLimiterType rateLimiterType,
                                                            size_t maxRequestsPerInterval,
                                                            std::chrono::milliseconds interval) {
    switch (rateLimiterType) {
    case RateLimiterType::FixedWindow:
        return std::make_unique<FixedWindowRateLimiter>(maxRequestsPerInterval, interval);
    case RateLimiterType::TokenBucket:
        return std::make_unique<TokenBucketRateLimiter>(maxRequestsPerInterval, interval);
    default:
        throw std::invalid_argument("Unknown RateLimiter type");
    }
}
