/*
 *  Copyright Howard Hinnant 2007-2010. Distributed under the Boost
 *  Software License, Version 1.0. (see http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef BOOST_SHARED_MUTEX_HPP
#define BOOST_SHARED_MUTEX_HPP

#include <climits>
#include <mutex>
#include <condition_variable>

namespace boost {

class shared_mutex {
public:
    shared_mutex() : state_(0) {}
    shared_mutex(const shared_mutex&) = delete;
    shared_mutex& operator=(const shared_mutex&) = delete;

    void lock() {
        std::unique_lock<std::mutex> lk(mut_);
        while (state_ & write_entered_) {
            gate1_.wait(lk);
        }
        state_ |= write_entered_;
        while (state_ & n_readers_) {
            gate2_.wait(lk);
        }
    }

    void unlock() {
        std::lock_guard<std::mutex> _(mut_);
        state_ = 0;
        gate1_.notify_all();
    }

    void lock_shared() {
        std::unique_lock<std::mutex> lk(mut_);
        while ((state_ & write_entered_) || (state_ & n_readers_) == n_readers_) {
            gate1_.wait(lk);
        }
        unsigned num_readers = (state_ & n_readers_) + 1;
        state_ &= ~n_readers_;
        state_ |= num_readers;
    }

    void unlock_shared() {
        unsigned num_readers = (state_ & n_readers_) - 1;
        state_ &= ~n_readers_;
        state_ |= num_readers;
        if (state_ & write_entered_) {
            if (num_readers == 0) {
                gate2_.notify_one();
            }
        } else {
            if (num_readers == n_readers_ - 1) {
                gate1_.notify_one();
            }
        }
    }

private:
    std::mutex mut_;
    std::condition_variable gate1_;
    std::condition_variable gate2_;
    unsigned state_;

    static const unsigned write_entered_ = 1U << (sizeof(unsigned)*CHAR_BIT - 1);
    static const unsigned n_readers_ = ~write_entered_;
};

}

#endif
