#include <chrono>
#include <format>
#include <iostream>
#include <memory>

#include "Constants.hpp"
#include "ObjectPool.hpp"
#include "Order.hpp"
#include "OrderBook.hpp"

int main() {
    std::cout << "--- NanoMatch HFT Engine ---\n\n";

    NanoMatch::ObjectPool<NanoMatch::Order> pool(NanoMatch::MAX_ORDERS);
    auto                                    book = std::make_unique<NanoMatch::OrderBook>();

    auto a1 = pool.allocate();
    a1->fill(1, 45000, 10, NanoMatch::Side::Sell);
    auto a2 = pool.allocate();
    a2->fill(2, 45001, 15, NanoMatch::Side::Sell);
    auto a3 = pool.allocate();
    a3->fill(3, 45002, 30, NanoMatch::Side::Sell);

    book->processOrder(a1);
    book->processOrder(a2);
    book->processOrder(a3);

    auto b1 = pool.allocate();
    b1->fill(4, 44999, 20, NanoMatch::Side::Buy);
    book->processOrder(b1);

    std::cout << "[Initial Book State]\n";
    std::cout << std::format("ASK 45002 : {}\n", book->getVolumeAt(NanoMatch::Side::Sell, 45002));
    std::cout << std::format("ASK 45001 : {}\n", book->getVolumeAt(NanoMatch::Side::Sell, 45001));
    std::cout << std::format("ASK 45000 : {}\n", book->getVolumeAt(NanoMatch::Side::Sell, 45000));
    std::cout << "-------------------\n";
    std::cout << std::format("BID 44999 : {}\n\n", book->getVolumeAt(NanoMatch::Side::Buy, 44999));

    book->cancelOrder(b1);
    pool.deallocate(b1);
    std::cout << "[Action] Bid Order 4 Canceled (O(1) tracking updated)\n\n";

    auto mo = pool.allocate();
    mo->fill(5, 0, 40, NanoMatch::Side::Buy);

    std::cout << "[Action] Injecting Market BUY Order (Qty: 40). Sweeping "
                 "asks...\n\n";

    auto start = std::chrono::high_resolution_clock::now();

    book->processMarketOrder(mo);

    auto end     = std::chrono::high_resolution_clock::now();
    auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    std::cout << std::format("[Performance] Sweep Latency: {} ns\n\n", latency);

    std::cout << "[Final Book State]\n";
    std::cout << std::format("ASK 45002 : {}\n", book->getVolumeAt(NanoMatch::Side::Sell, 45002));
    std::cout << std::format("ASK 45001 : {}\n", book->getVolumeAt(NanoMatch::Side::Sell, 45001));
    std::cout << std::format("ASK 45000 : {}\n", book->getVolumeAt(NanoMatch::Side::Sell, 45000));
    std::cout << "-------------------\n";
    std::cout << std::format("BID 44999 : {}\n\n", book->getVolumeAt(NanoMatch::Side::Buy, 44999));

    return 0;
}
