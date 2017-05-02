#include <atomic>

/* TODO, inline functions for perf */

/* Useful in a spin lock to pause 
 * Apparently, a pause causes an abort... 
 * I am taking this out for now, but when waiting on 
 * the spin lock, we are already in abort handler code*/

//#define pause() asm volatile("pause" ::: "memory")

class spin_lock{

    private:
        int val;

    public: 
        spin_lock(): val(0){ }

        inline void acquire() {
            while(__sync_lock_test_and_set(&val,1)){
                //Nothing
            }
        }

        inline void spin_until_free(){
            while (val==1)
                ;
        }

        inline void release() {
            __sync_synchronize();
            val = 0;
        }

        inline bool held() {
            return val==1;
        }

        /*
        bool held_relaxed() {
            return tm_lock.load(std::memory_order_relaxed);
        }
        */

};
