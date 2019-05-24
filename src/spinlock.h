#pragma once

#include <thread>

class spinlock_t {
    std::atomic_flag lock_flag;
public:
    spinlock_t() { lock_flag.clear(); }

    bool try_lock() { return !lock_flag.test_and_set(std::memory_order_acquire); }
    void lock() { for (size_t i = 0; !try_lock(); ++i) if (i % 100 == 0) std::this_thread::yield(); }
    void unlock() { lock_flag.clear(std::memory_order_release); }
//    void unlock_shared() { lock(); }
//    bool try_lock_shared() { try_lock(); }
};