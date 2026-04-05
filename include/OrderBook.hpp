#pragma once
#include <array>
#include <bit>
#include <cstdint>

#include "Constants.hpp"
#include "Order.hpp"
#include "PriceLevel.hpp"

namespace NanoMatch {
class OrderBook {
  public:
    bool processMarketOrder(Order *mo) {
        bool executedAny = false;

        if (mo->side == Side::Buy) {
            while (mo->quantity > 0 && m_bestAsk != UINT64_MAX) {
                Order tempAggressive;
                tempAggressive.quantity = mo->quantity;
                tempAggressive.price    = m_bestAsk;
                matchAggressiveOrder(&tempAggressive, m_asks, Side::Sell);
                mo->quantity = tempAggressive.quantity;
                executedAny  = true;
            }
        } else {
            while (mo->quantity > 0 && m_bestBid != 0) {
                Order tempAggressive;
                tempAggressive.quantity = mo->quantity;
                tempAggressive.price    = m_bestBid;
                matchAggressiveOrder(&tempAggressive, m_bids, Side::Buy);
                mo->quantity = tempAggressive.quantity;
                executedAny  = true;
            }
        }

        return executedAny;
    }

    bool processOrder(Order *incomingOrder) {
        if (!isValidPrice(incomingOrder->price))
            return false;
        if (incomingOrder->quantity == 0)
            return false;

        if (incomingOrder->side == Side::Buy) {
            matchAggressiveOrder(incomingOrder, m_asks, Side::Sell);
        } else {
            matchAggressiveOrder(incomingOrder, m_bids, Side::Buy);
        }

        if (incomingOrder->quantity > 0) {
            addOrder(incomingOrder);
        }

        return true;
    }

    // O(1) Deterministic cancellation utilizing intrusive pointers to bypass
    // linear search.
    void cancelOrder(Order *orderToCancel) {
        PriceLevel& level = getLevel(orderToCancel->side, orderToCancel->price);
        level.removeOrder(orderToCancel);
        orderToCancel->quantity = 0;

        if (level.totalVolume == 0) {
            size_t index = (orderToCancel->price - MIN_PRICE) / TICK_SIZE;
            clearBit(orderToCancel->side, index);
            updateBestPriceOnRemove(orderToCancel->side, orderToCancel->price);
        }
    }

    uint32_t getVolumeAt(Side side, uint64_t price) {
        return getLevel(side, price).totalVolume;
    }

  private:
    std::array<PriceLevel, MAX_PRICE_TICKS> m_bids;
    std::array<PriceLevel, MAX_PRICE_TICKS> m_asks;

    std::array<uint64_t, NUM_BIT_WORDS> m_bid_bits = {0};
    std::array<uint64_t, NUM_BIT_WORDS> m_ask_bits = {0};

    uint64_t m_bestBid = 0;
    uint64_t m_bestAsk = UINT64_MAX;

    void setBit(Side side, size_t index) {
        size_t word_idx = index / 64;
        size_t bit_idx  = index % 64;
        if (side == Side::Buy)
            m_bid_bits[word_idx] |= (1ULL << bit_idx);
        else
            m_ask_bits[word_idx] |= (1ULL << bit_idx);
    }

    void clearBit(Side side, size_t index) {
        size_t word_idx = index / 64;
        size_t bit_idx  = index % 64;
        if (side == Side::Buy)
            m_bid_bits[word_idx] &= ~(1ULL << bit_idx);
        else
            m_ask_bits[word_idx] &= ~(1ULL << bit_idx);
    }

    // O(1) Hardware intrinsic bit-scanning (TZCNT/BSR) for instant best-price
    // discovery.
    void updateBestPriceOnAdd(Side side, uint64_t price) {
        if (side == Side::Buy && price > m_bestBid) {
            m_bestBid = price;
        } else if (side == Side::Sell && price < m_bestAsk) {
            m_bestAsk = price;
        }
    }

    void updateBestPriceOnRemove(Side side, uint64_t price) {
        if (side == Side::Buy && price == m_bestBid) {
            for (int i = NUM_BIT_WORDS - 1; i >= 0; --i) {
                if (m_bid_bits[i] != 0) {
                    size_t bit_idx = 63 - std::countl_zero(m_bid_bits[i]);
                    m_bestBid      = MIN_PRICE + ((i * 64 + bit_idx) * TICK_SIZE);
                    return;
                }
            }
            m_bestBid = 0;
        } else if (side == Side::Sell && price == m_bestAsk) {
            for (size_t i = 0; i < NUM_BIT_WORDS; ++i) {
                if (m_ask_bits[i] != 0) {
                    size_t bit_idx = std::countr_zero(m_ask_bits[i]);
                    m_bestAsk      = MIN_PRICE + ((i * 64 + bit_idx) * TICK_SIZE);
                    return;
                }
            }
            m_bestAsk = UINT64_MAX;
        }
    }

    void matchAggressiveOrder(Order *aggressiveOrder, std::array<PriceLevel, MAX_PRICE_TICKS>& oppositeBook,
                              Side oppositeSide) {
        size_t      targetIndex = (aggressiveOrder->price - MIN_PRICE) / TICK_SIZE;
        PriceLevel& targetLevel = oppositeBook[targetIndex];

        if (targetLevel.totalVolume == 0)
            return;

        Order *restingOrder = targetLevel.head;

        while (restingOrder != nullptr && aggressiveOrder->quantity > 0) {
            uint32_t tradeQuantity = std::min((uint32_t)aggressiveOrder->quantity, (uint32_t)restingOrder->quantity);

            aggressiveOrder->quantity -= tradeQuantity;
            restingOrder->quantity -= tradeQuantity;
            targetLevel.totalVolume -= tradeQuantity;

            if (restingOrder->quantity == 0) {
                Order *orderToRemove = restingOrder;
                restingOrder         = restingOrder->next;
                targetLevel.removeOrder(orderToRemove);
            } else {
                break;
            }
        }

        if (targetLevel.totalVolume == 0) {
            clearBit(oppositeSide, targetIndex);
            updateBestPriceOnRemove(oppositeSide, aggressiveOrder->price);
        }
    }

    bool addOrder(Order *order) {
        if (!isValidPrice(order->price))
            return false;

        size_t index = (order->price - MIN_PRICE) / TICK_SIZE;

        PriceLevel& level = (order->side == Side::Buy) ? m_bids[index] : m_asks[index];

        if (level.price == 0) {
            level.price = order->price;
        }

        level.addOrder(order);

        setBit(order->side, index);
        updateBestPriceOnAdd(order->side, order->price);

        return true;
    }

    // O(1) Bounds check
    bool isValidPrice(uint64_t price) const {
        return (price >= MIN_PRICE) && (((price - MIN_PRICE) / TICK_SIZE) < MAX_PRICE_TICKS);
    }

    // O(1) Cache-friendly flat array lookup resolving raw price to memory block
    // index.
    PriceLevel& getLevel(Side side, uint64_t price) {
        size_t index = (price - MIN_PRICE) / TICK_SIZE;
        if (side == Side::Buy)
            return m_bids[index];
        else
            return m_asks[index];
    }
};
} // namespace NanoMatch
