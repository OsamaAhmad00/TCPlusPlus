#pragma once

#include <condition_variable>
#include <queue>

namespace tcpp {

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

template <typename Callback>
requires requires(Callback cb) { cb(); }
struct Timer {
    TimePoint time;
    Callback callback;

    friend constexpr auto operator<=>(const Timer& lhs, const Timer &rhs) {
        return lhs.time <=> rhs.time;
    }
};

template <typename Callback>
requires requires(Callback cb) { cb(); }
class TimersManager {

    std::mutex m;  // TODO change this
    std::priority_queue<Timer<Callback>, std::vector<Timer<Callback>>, std::greater<>> timers;
    std::condition_variable cv;
    std::jthread handler_thread { &TimersManager::handler, this };

    bool should_execute(const Timer<Callback>& timer) {
        // True if the timer is about to expire in less than or equal
        //  to 10 milliseconds or should've been already executed
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(timer.time - now);
        return duration.count() <= 10;
    }

    void handler(std::stop_token token) {
        std::unique_lock lock(m);
        while (!token.stop_requested()) {
            if (timers.empty()) {
                // The only reason for a wake-up is that something is put into the queue
                cv.wait(lock);
            }

            if (!timers.empty()) {
                if (should_execute(timers.top())) {
                    // Woke up because of a timer expiration
                    timers.top().callback();
                    timers.pop();
                } else {
                    // Woke up because an earlier timer is inserted
                    cv.wait_until(lock, timers.top().time);
                }
            } else {
                // Woken up because the thread needs to terminate
            }
        }
    }

public:

    TimersManager() = default;

    TimersManager(const TimersManager&) = delete;

    TimersManager& operator=(const TimersManager&) = delete;

    TimersManager(TimersManager&&) = delete;

    TimersManager& operator=(TimersManager&&) = delete;

    void addTimer(Timer<Callback> timer) {
        std::lock_guard lock(m);
        bool need_to_update_wait_time = !timers.empty() && (timer.time < timers.top().time);
        timers.push(std::move(timer));
        if (need_to_update_wait_time) {
            cv.notify_one();
        }
    }

    ~TimersManager() noexcept {
        cv.notify_one();
    }
};

}