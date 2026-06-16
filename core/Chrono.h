#pragma once

#include <chrono>

class Timer
{
public:
    Timer()
        : _start(std::chrono::steady_clock::now())
    {
    }

    void reset()
    {
        _start = std::chrono::steady_clock::now();
    }

    struct Duration
    {
        double seconds;

        double to_secs() const
        {
            return seconds;
        }
    };

    Duration elapsed() const
    {
        return {
            std::chrono::duration<double>(
                std::chrono::steady_clock::now() - _start
            ).count()
        };
    }

private:
    std::chrono::steady_clock::time_point _start;
};