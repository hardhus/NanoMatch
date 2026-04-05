#pragma once
#include <cstddef>
#include <vector>

namespace NanoMatch {
template <typename T> class ObjectPool {
  public:
    explicit ObjectPool(size_t size) {
        m_pool.resize(size);
        m_free_pointers.reserve(size);

        for (size_t i = 0; i < size; ++i) {
            m_free_pointers.push_back(&m_pool[i]);
        }
    }

    // Zero-allocation: Retrieves pre-allocated memory from the contiguous pool
    // to bypass OS heap locks.
    [[nodiscard]] T *allocate() {
        if (m_free_pointers.empty()) {
            return nullptr;
        }
        T *obj = m_free_pointers.back();
        m_free_pointers.pop_back();
        return obj;
    }

    void deallocate(T *obj) {
        m_free_pointers.push_back(obj);
    }

  private:
    std::vector<T>   m_pool;
    std::vector<T *> m_free_pointers;
};
} // namespace NanoMatch
