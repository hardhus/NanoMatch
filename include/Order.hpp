#pragma once
#include <cstdint>

namespace NanoMatch {
enum class Side : uint8_t { Buy, Sell };

struct Order {
    uint64_t id;
    uint64_t price;
    uint64_t quantity;
    Side     side;

    Order *next = nullptr;
    Order *prev = nullptr;

    void fill(uint64_t _id, uint64_t _price, uint32_t _qty, Side _side) {
        id       = _id;
        price    = _price;
        quantity = _qty;
        side     = _side;
        next     = nullptr;
        prev     = nullptr;
    }
};
} // namespace NanoMatch
