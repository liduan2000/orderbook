#pragma once

#include <mutex>
#include <queue>

#include "rate_limiter.hpp"

class FixedWindowRateLimiter : public RateLimiter {
  public:
    FixedWindowRateLimiter(size_t maxRequests = 10, std::chrono::milliseconds interval = std::chrono::seconds(1));

    bool checkRequestRate() override;

    void reset() override;

  private:
    std::mutex requestRateMutex_;
    std::queue<std::chrono::steady_clock::time_point> requestTimestamps_;
};
