#include "TokenBucketRateLimiter.h"

TokenBucketRateLimiter::TokenBucketRateLimiter(size_t maxRequests, std::chrono::milliseconds interval)
    : RateLimiter(maxRequests, interval) {
    if (maxRequests == 0) {
        throw std::invalid_argument("Max requests must be greater than 0");
    }
    if (interval.count() <= 0) {
        throw std::invalid_argument("Interval must be greater than 0 milliseconds");
    }

    bucketWidthMs_ = std::max(static_cast<size_t>(RATE_LIMIT_INTERVAL.count() / 10), static_cast<size_t>(1));
    bucketCount_ = static_cast<size_t>(std::ceil(static_cast<double>(RATE_LIMIT_INTERVAL.count()) / bucketWidthMs_));
    buckets_ = std::make_unique<std::atomic<size_t>[]>(bucketCount_);
}

bool TokenBucketRateLimiter::checkRequestRate() {
    auto now = std::chrono::steady_clock::now();
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    size_t currentBucket = (nowMs / bucketWidthMs_) % bucketCount_;
    rotateBuckets(nowMs);

    return tryAddRequest(currentBucket);
}

bool TokenBucketRateLimiter::tryAddRequest(size_t currentBucket) {
    const int maxSpins = 100;
    int spinCount = 0;
    if (totalRequests_.load(std::memory_order_relaxed) >= MAX_REQUESTS) {
        return false;
    }

    size_t expected = buckets_[currentBucket].load(std::memory_order_relaxed);
    while (!buckets_[currentBucket].compare_exchange_weak(expected, expected + 1, std::memory_order_acq_rel,
                                                          std::memory_order_relaxed)) {
        if (totalRequests_.load(std::memory_order_relaxed) >= MAX_REQUESTS || ++spinCount > maxSpins) {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            return false;
        }
    }

    totalRequests_.fetch_add(1, std::memory_order_acq_rel);
    return true;
}

void TokenBucketRateLimiter::rotateBuckets(int64_t nowMs) {
    int64_t lastTime = lastRotateTime_.load(std::memory_order_relaxed);
    int64_t elapsed = nowMs - lastTime;

    if (static_cast<size_t>(elapsed) < bucketWidthMs_) {
        return;
    }

    if (lastRotateTime_.compare_exchange_strong(lastTime, nowMs, std::memory_order_acq_rel)) {
        size_t bucketsToClear = std::min(static_cast<size_t>(elapsed / bucketWidthMs_), bucketCount_);

        for (size_t i = 0; i < bucketsToClear; ++i) {
            size_t bucketIndex = (lastTime / bucketWidthMs_ + i) % bucketCount_;
            if (static_cast<int64_t>(nowMs - bucketIndex * bucketWidthMs_) >= RATE_LIMIT_INTERVAL.count()) {
                size_t cleared = buckets_[bucketIndex].load(std::memory_order_relaxed);
                buckets_[bucketIndex].store(0, std::memory_order_relaxed);
                totalRequests_.fetch_sub(cleared, std::memory_order_acq_rel);
            }
        }
    }
}

void TokenBucketRateLimiter::reset() {
    std::lock_guard<std::mutex> lock(resetMutex_);

    for (size_t i = 0; i < bucketCount_; ++i) {
        buckets_[i].store(0, std::memory_order_relaxed);
    }
    totalRequests_.store(0, std::memory_order_relaxed);
    lastRotateTime_.store(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
            .count(),
        std::memory_order_relaxed);
}
