# NanoMatch: Ultra-Low Latency HFT Matching Engine

NanoMatch is a ultra-low latency, deterministic financial matching engine designed for High-Frequency Trading (HFT) environments. Its architecture maximizes throughput and ensures predictable execution paths by targeting zero runtime heap allocations, flat-array lookups, and hardware-accelerated processing loops.

## 🚀 Key Architectural Features

* **Zero-Allocation Object Pool (`ObjectPool`):** Pre-allocates memory for structures upfront in a contiguous vector container. This design completely completely bypasses OS heap resource locks during hot-path execution states to avoid standard system latency fluctuations.
* **Intrusive Double-Linked Lists (`PriceLevel`):** Stores memory pointer chains (`next`/`prev`) inside the raw `Order` structure. This eliminates node wrapper allocation overheads and achieves true $O(1)$ algorithmic complexity during placement, matching or cancellation requests.
* **Hardware Intrinsic Bit-Scanning (Bitmaps):** Maps bid and ask positions across pre-allocated bit arrays (`NUM_BIT_WORDS`). Using modern CPU intrinsics (`std::countl_zero` and `std::countr_zero`) allows the engine to query best bid or best ask data paths instantly in a deterministic $O(1)$ timeframe, avoiding any loop indexing operations.
* **Cache-Friendly Flat Array Lookups:** Maps absolute currency ranges to precise table indexing boundaries through immediate mathematical transformations.

## 📂 Project Structure

* **`include/Constants.hpp`**: Compilation-time metrics tuning maximum queue capacities, minimum tick offsets, and bit array scales.
* **`include/ObjectPool.hpp`**: Fixed-size memory block pre-allocator preventing operating system memory locks.
* **`include/Order.hpp`**: Fundamental intrusive entity matching identities, trade side parameters, and node direction chains.
* **`include/PriceLevel.hpp`**: Price queue block maintaining entry sequence processing rules and current limits volume.
* **`include/OrderBook.hpp`**: Central transaction execution pipeline driving aggressive sweeping, bitmask toggling, and cancellations.
* **`src/main.cpp`**: Latency driver module simulating typical limit matching contexts and logging precise performance sweeps.

## 🛠️ Build & Prerequisites

The codebase is written using standard features requiring a modern environment compliant with the **C++20** standard (e.g., GCC 11+, Clang 13+, or MSVC 2022+). It utilizes the `<bit>`, `<chrono>`, and `<format>` library extensions.

### Manual Compilation

```bash
g++ -O3 -std=c++20 src/main.cpp -Iinclude -o nanomatch

```
