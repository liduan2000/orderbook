#include "fixed_window_rate_limiter.hpp"

FixedWindowRateLimiter::FixedWindowRateLimiter(size_t maxRequests, std::chrono::milliseconds interval)
    : RateLimiter(maxRequests, interval) {}

bool FixedWindowRateLimiter::checkRequestRate() {
    std::lock_guard<std::mutex> lock(requestRateMutex_);

    auto now = std::chrono::steady_clock::now();
    // std::cout << "Elapsed time: "
    //           << std::chrono::duration_cast<std::chrono::milliseconds>(now - requestTimestamps_.front()).count()
    //           << "ms, current window requests: " << requestTimestamps_.size() << std::endl;
    while (!requestTimestamps_.empty() && (now - requestTimestamps_.front()) > RATE_LIMIT_INTERVAL) {
        requestTimestamps_.pop();
    }
    if (requestTimestamps_.size() >= MAX_REQUESTS) { return false; }

    requestTimestamps_.push(now);
    return true;
}

void FixedWindowRateLimiter::reset() {
    std::lock_guard<std::mutex> lock(requestRateMutex_);
    std::queue<std::chrono::steady_clock::time_point> emptyQueue;
    std::swap(requestTimestamps_, emptyQueue);
}