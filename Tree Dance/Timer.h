/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

#pragma once
#include <cstdint>
#include <chrono>

class Timer {
 public:
    Timer();
    void Tick();
    double GetElapsed(bool averaged = false);

 private:
    std::chrono::high_resolution_clock::time_point last_tick_{};
    std::chrono::high_resolution_clock::time_point last_tick_avg_{};
    double seconds_per_frame_{};
    double avg_seconds_per_frame_{};
    double time_since_average_{};
    uint64_t frames_since_average_{};
};
