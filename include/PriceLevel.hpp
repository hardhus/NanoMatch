#pragma once
#include <cstdint>

#include "Order.hpp"

namespace NanoMatch {
class PriceLevel {
  public:
    uint64_t price       = 0;
    uint32_t totalVolume = 0;

    Order *head = nullptr;
    Order *tail = nullptr;

    // O(1) Intrusive list manipulation: Eliminates node allocation overhead for
    // resting orders.
    void addOrder(Order *order) {
        if (!head) {
            head = tail = order;
        } else {
            tail->next  = order;
            order->prev = tail;
            tail        = order;
        }
        totalVolume += order->quantity;
    }

    void removeOrder(Order *order) {
        if (order->prev) {
            order->prev->next = order->next;
        } else {
            head = order->next;
        }

        if (order->next) {
            order->next->prev = order->prev;
        } else {
            tail = order->prev;
        }

        order->next = nullptr;
        order->prev = nullptr;
        totalVolume -= order->quantity;
    }
};
} // namespace NanoMatch
