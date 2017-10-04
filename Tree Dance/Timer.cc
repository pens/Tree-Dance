/*
    Copyright (c) 2015 Seth Pendergrass. See LICENSE.
*/

#include "Timer.h"
using namespace std;
using namespace chrono;

Timer::Timer() {
    last_tick_ = high_resolution_clock::now();
}

void Timer::Tick() {
    auto current_tick = high_resolution_clock::now();
    seconds_per_frame_ =
        duration_cast<duration<double>>(current_tick - last_tick_).count();
    last_tick_ = current_tick;

    time_since_average_ += seconds_per_frame_;
    ++frames_since_average_;
    if (time_since_average_ > 1.0) {
        avg_seconds_per_frame_ = duration_cast<duration<double>>(
            current_tick - last_tick_avg_).count() / frames_since_average_;

        time_since_average_ = 0.0;
        frames_since_average_ = 0;
        last_tick_avg_ = current_tick;
    }
}

double Timer::GetElapsed(bool averaged) {
    return averaged ? avg_seconds_per_frame_ : seconds_per_frame_;
}

