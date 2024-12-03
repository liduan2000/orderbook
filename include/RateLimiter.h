#ifndef RATELIMETER_H
#define RATELIMETER_H

#include <chrono>
#include <queue>

class RateLimiter {
   public:
    RateLimiter(size_t maxRequests, std::chrono::milliseconds interval)
        : MAX_REQUESTS(maxRequests), RATE_LIMIT_INTERVAL(interval) {}

    virtual ~RateLimiter() = default;

    virtual bool checkRequestRate() = 0;

    virtual void reset() = 0;

   protected:
    const size_t MAX_REQUESTS;
    const std::chrono::milliseconds RATE_LIMIT_INTERVAL;
};

#endif  // RATELIMETER_H