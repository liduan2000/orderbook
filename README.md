# Core Trading System Modules: OrderBook and RiskControl

- **OrderBook**:  
  A high-performance OrderBook module leveraging efficient data structures for fast order CRUD operations.

  - Utilized read-write locks to ensure thread-safe operations in a concurrent environment.
  - Optimized for high-frequency trading scenarios, achieving exceptional performance and data consistency.

- **RiskControl**:  
  Two critical submodules to enhance system reliability and manage trading risks:
  - **SelfCrossChecker**: Prevented self-crossing by validating new orders against existing orders in the system.
  - **RateLimiter**: Two custom rate-limiting algorithms:
    - **Sliding Window**: Managed request rates with thread-safe synchronization using mutex locks.
    - **Token Bucket**: Combined atomic operations, spinlocks, and mutexes to efficiently control request flow in multi-threaded environments.

# Getting Started

## Make and Run Tests

```
make
./test.exe
```

## Clean

```
make clean
```
