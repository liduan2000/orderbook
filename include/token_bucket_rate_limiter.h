#ifndef TOKENBUCKETRATELIMITER_H
#define TOKENBUCKETRATELIMITER_H

#include <atomic>
#include <cmath>
#include <mutex>

#include "rate_limiter.h"

class TokenBucketRateLimiter : public RateLimiter {
  public:
    TokenBucketRateLimiter(size_t maxRequests = 10, std::chrono::milliseconds interval = std::chrono::seconds(1));

    bool checkRequestRate() override;

    void reset() override;

  private:
    bool tryAddRequest(size_t currentBucket);

    void rotateBuckets(int64_t nowMs);

    size_t bucketWidthMs_;
    size_t bucketCount_;
    std::unique_ptr<std::atomic<size_t>[]> buckets_;
    std::atomic<int64_t> lastRotateTime_{0};
    std::atomic<size_t> totalRequests_{0};
    std::mutex resetMutex_;
};

#endif // TOKENBUCKETRATELIMITER_H
