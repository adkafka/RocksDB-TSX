#include <atomic>

/* TODO, inline functions for perf */

/* Useful in a spin lock to pause 
 * Apparently, a pause causes an abort... so don't use this*/
//#define pause() asm volatile("pause" ::: "memory")

class spin_lock{

    private:
        std::atomic<bool> tm_lock;

    public: 
        spin_lock(): tm_lock(false){ }

        void acquire() {
            do {
                spin_until_free();
            } while (tm_lock.exchange(true));
        }

        void spin_until_free(){
            while (tm_lock.load())
                ;
        }

        void release() {
            tm_lock.store(false, std::memory_order_release);
        }

        bool held() {
            return tm_lock.load();
        }

        bool held_relaxed() {
            return tm_lock.load(std::memory_order_relaxed);
        }

};
