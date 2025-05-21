#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>

#include "orderbook.hpp"
#include "risk_control.hpp"

// Comprehensive Test Suite
class OrderBookTest {
  public:
    static void runAllTests() {
        testBasicOrderPlacement();
        testSelfCrossDetection();
        testRequestRateLimit(RiskControl::RateLimiterType::FixedWindow);
        testRequestRateLimit(RiskControl::RateLimiterType::TokenBucket);
        testSelfCrossAndRateLimit(RiskControl::RateLimiterType::FixedWindow);
        testSelfCrossAndRateLimit(RiskControl::RateLimiterType::TokenBucket);
        // low concurrency
        testRateLimitForConcurrentOrderPlacement(RiskControl::RateLimiterType::FixedWindow, 1, 100);
        // high concurrency
        testRateLimitForConcurrentOrderPlacement(RiskControl::RateLimiterType::FixedWindow, 100, 20);
        // low concurrency
        testRateLimitForConcurrentOrderPlacement(RiskControl::RateLimiterType::TokenBucket, 1, 100);
        // high concurrency
        testRateLimitForConcurrentOrderPlacement(RiskControl::RateLimiterType::TokenBucket, 100, 20);
        std::cout << "All tests passed." << std::endl;
        std::cout << "You can go and have a good sleep:)" << std::endl;
    }

  private:
    // Test basic order placement
    static void testBasicOrderPlacement() {
        OrderBook orderBook;
        RiskControl riskControl(orderBook);

        Order buyOrder("1", "AAPL", 150.0, 100, true);
        Order sellOrder("2", "AAPL", 151.0, 100, false);

        std::string rejectReason;
        bool approved = riskControl.approveNewOrder(&buyOrder, rejectReason);
        assert(approved == true);
        assert(rejectReason == "");
        orderBook.addOrder(buyOrder);

        approved = riskControl.approveNewOrder(&sellOrder, rejectReason);
        assert(approved == true);
        assert(rejectReason == "");
        orderBook.addOrder(sellOrder);

        Order* order = nullptr;
        approved = riskControl.approveNewOrder(order, rejectReason);
        assert(approved == false);
        assert(rejectReason == "Invalid order pointer");

        std::cout << "Basic order placement test passed." << std::endl;
    }

    // Test self-cross detection
    static void testSelfCrossDetection() {
        OrderBook orderBook;
        RiskControl riskControl(orderBook);

        Order buyOrder1("1", "AAPL", 150.0, 100, true);
        Order sellOrder1("2", "AAPL", 151.0, 100, false);
        Order buyOrder2("3", "AAPL", 151.5, 100, true);   // Self-cross
        Order sellOrder2("4", "AAPL", 149.5, 100, false); // Self-cross

        std::string rejectReason;
        bool approved = riskControl.approveNewOrder(&buyOrder1, rejectReason);
        assert(approved == true);
        assert(rejectReason.empty());
        orderBook.addOrder(buyOrder1);

        approved = riskControl.approveNewOrder(&sellOrder1, rejectReason);
        assert(approved == true);
        assert(rejectReason.empty());
        orderBook.addOrder(sellOrder1);

        approved = riskControl.approveNewOrder(&buyOrder2, rejectReason);
        assert(approved == false);
        assert(rejectReason == "Self-cross detected");

        approved = riskControl.approveNewOrder(&sellOrder2, rejectReason);
        assert(approved == false);
        assert(rejectReason == "Self-cross detected");

        std::cout << "Self cross detection test passed." << std::endl;
    }

    // Test request rate limiting
    static void testRequestRateLimit(RiskControl::RateLimiterType ratelimiterType) {
        OrderBook orderBook;
        RiskControl riskControl(orderBook, ratelimiterType, 5,
                                std::chrono::milliseconds(1000)); // 10 requests per second
        // Place 3 orders quickly
        std::string rejectReason;
        for (int i = 0; i < 5; i++) {
            Order buyOrder(std::to_string(i + 1), "AAPL", 150.0, 100, true);
            bool approved = riskControl.approveNewOrder(&buyOrder, rejectReason);
            assert(approved == true);
            assert(rejectReason.empty());
            orderBook.addOrder(buyOrder);
        }

        // 6th order should be rejected due to rate limit
        Order buyOrder("6", "AAPL", 150.0, 100, true);
        bool approved = riskControl.approveNewOrder(&buyOrder, rejectReason);
        assert(approved == false);
        assert(rejectReason == "Request rate exceeded");

        // Simulate a delay and check if request limit resets
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Now the request should pass as the rate limit has reset
        approved = riskControl.approveNewOrder(&buyOrder, rejectReason);
        assert(approved == true);
        assert(rejectReason.empty());
        orderBook.addOrder(buyOrder);

        if (ratelimiterType == RiskControl::RateLimiterType::FixedWindow) {
            std::cout << "Fixed Window Rate Limiter: ";
        } else if (ratelimiterType == RiskControl::RateLimiterType::TokenBucket) {
            std::cout << "Token Bucket Rate Limiter: ";
        } else {
            std::cout << "Unknown Rate Limiter Type: ";
        }
        std::cout << "Request rate limiting test passed." << std::endl;
    }

    // Test self-cross detection & rate limiting
    static void testSelfCrossAndRateLimit(RiskControl::RateLimiterType ratelimiterType) {
        OrderBook orderBook;
        RiskControl riskControl(orderBook, ratelimiterType, 3,
                                std::chrono::milliseconds(1000)); // 3 requests per second

        Order buyOrder1("1", "AAPL", 150.0, 100, true);
        Order sellOrder1("2", "AAPL", 151.0, 100, false);
        Order buyOrder2("3", "AAPL", 149.0, 100, true);
        Order buyOrder3("4", "AAPL", 148.0, 100, true);
        Order sellOrder2("5", "AAPL", 149.5, 100, false); // Self-cross

        std::string rejectReason;
        bool approved = riskControl.approveNewOrder(&buyOrder1, rejectReason);
        assert(approved == true);
        assert(rejectReason.empty());
        orderBook.addOrder(buyOrder1);

        approved = riskControl.approveNewOrder(&sellOrder1, rejectReason);
        assert(approved == true);
        assert(rejectReason.empty());
        orderBook.addOrder(sellOrder1);

        approved = riskControl.approveNewOrder(&buyOrder2, rejectReason);
        assert(approved == true);
        assert(rejectReason.empty());

        approved = riskControl.approveNewOrder(&buyOrder3, rejectReason);
        assert(approved == false);
        assert(rejectReason == "Request rate exceeded");

        approved = riskControl.approveNewOrder(&sellOrder2, rejectReason);
        assert(approved == false);
        assert(rejectReason == "Self-cross detected");

        if (ratelimiterType == RiskControl::RateLimiterType::FixedWindow) {
            std::cout << "Fixed Window Rate Limiter: ";
        } else if (ratelimiterType == RiskControl::RateLimiterType::TokenBucket) {
            std::cout << "Token Bucket Rate Limiter: ";
        } else {
            std::cout << "Unknown Rate Limiter Type: ";
        }
        std::cout << "Self cross & rate limiting combination test passed." << std::endl;
    }

    // Test rate limiting for concurrent order placement
    static void testRateLimitForConcurrentOrderPlacement(RiskControl::RateLimiterType ratelimiterType,
                                                         int numThreads = 100, int requestsPerThread = 1) {
        OrderBook orderBook;
        RiskControl riskControl(orderBook, ratelimiterType, 10,
                                std::chrono::milliseconds(1000)); // 10 req per second

        std::vector<std::thread> threads;
        std::atomic<int> successCount(0);
        std::atomic<int> rejectCount(0);

        for (int t = 0; t < numThreads; ++t) {
            threads.emplace_back([&orderBook, &riskControl, &successCount, &rejectCount, t, requestsPerThread]() {
                for (int i = 0; i < requestsPerThread; ++i) {
                    int orderId = t * requestsPerThread + i;
                    Order order(std::to_string(orderId), "AAPL", 150.0 + (orderId % 10), 10, (orderId % 2 == 0));

                    std::string rejectReason;
                    bool approved = riskControl.approveNewOrder(&order, rejectReason);
                    if (approved) {
                        orderBook.addOrder(order);
                        successCount.fetch_add(1, std::memory_order_relaxed);
                    } else {
                        rejectCount.fetch_add(1, std::memory_order_relaxed);
                    }
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                }
            });
        }

        for (auto& thread : threads) { thread.join(); }

        // assert requests
        int totalOrders = successCount + rejectCount;
        assert(totalOrders == numThreads * requestsPerThread);
        assert(successCount <= numThreads * requestsPerThread);
        assert(rejectCount >= 0);

        if (ratelimiterType == RiskControl::RateLimiterType::FixedWindow) {
            std::cout << "Fixed Window Rate Limiter: ";
        } else if (ratelimiterType == RiskControl::RateLimiterType::TokenBucket) {
            std::cout << "Token Bucket Rate Limiter: ";
        } else {
            std::cout << "Unknown Rate Limiter Type: ";
        }

        std::cout << "Concurrent order placement test passed.\n";
    }
};

int main() {
    OrderBookTest::runAllTests();
    return 0;
}
