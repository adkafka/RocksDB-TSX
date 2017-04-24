#include <atomic>

class spin_lock{

    private:
        std::atomic<bool> tm_lock;

    public: 
        spin_lock(): tm_lock(0){ }

        void acquire() {
            do {
                while (tm_lock.load()) { }
            } while (tm_lock.exchange(true));
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
