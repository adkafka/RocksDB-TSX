#include <atomic>

/* TODO, inline functions for perf */

/* Useful in a spin lock to pause */
#define pause() asm volatile("pause" ::: "memory")

class spin_lock{

    private:
        std::atomic<bool> tm_lock;

    public: 
        spin_lock(): tm_lock(false){ }

        inline void acquire() {
            do {
                spin_until_free();
            } while (tm_lock.exchange(true));
        }

        inline void spin_until_free(){
            while (tm_lock.load())
                pause();
        }

        inline void release() {
            tm_lock.store(false, std::memory_order_release);
        }

        inline bool held() {
            return tm_lock.load();
        }

        inline bool held_relaxed() {
            return tm_lock.load(std::memory_order_relaxed);
        }

};
