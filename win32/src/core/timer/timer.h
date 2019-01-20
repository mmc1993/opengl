#pragma once

#include "../include.h"

class Timer {
private:
    static size_t s_countID;

public:
    struct Task {
        size_t mID;
        std::function<void()> mCall;
        std::chrono::time_point<std::chrono::high_resolution_clock> mTime;

        Task()
        { }

        Task(size_t id, 
            const std::function<void()> & call, 
            const std::chrono::time_point<std::chrono::high_resolution_clock> & time)
            : mID(id), mCall(call), mTime(time)
        { }

        bool operator ==(size_t id) const
        {
            return mID == id;
        }

        bool operator <(const Timer::Task & other) const
        {
            return mTime < other.mTime;
        }

        bool operator >(const Timer::Task & other) const
        {
            return mTime > other.mTime;
        }

        bool operator ==(const Timer::Task & other) const
        {
            return mTime == other.mTime;
        }

        bool operator !=(const Timer::Task & other) const
        {
            return mTime != other.mTime;
        }
    };

public:
    Timer()
    { }

    ~Timer()
    { }
    
    void Update(const std::chrono::time_point<std::chrono::high_resolution_clock> & time)
    {
        while (!_tasks.empty() && _tasks.front().mTime <= time)
        {
			auto fn = _tasks.front().mCall;
            std::pop_heap(
                std::begin(_tasks), 
                std::end(_tasks));
            _tasks.pop_back();
			fn();
        }
    }

    size_t Add(size_t ms, const std::function<void()> & func)
    {
        _tasks.emplace_back(Timer::s_countID, func, 
            std::chrono::high_resolution_clock::now() + 
            std::chrono::milliseconds(ms));
        std::push_heap(_tasks.begin(), _tasks.end());
        return Timer::s_countID++;
    }

    void Del(size_t id)
    {
        auto it = std::find(std::begin(_tasks), std::end(_tasks), id);
        if (it != std::end(_tasks))
        {
            _tasks.erase(it);
            std::sort_heap(
                std::begin(_tasks), 
                std::end(_tasks));
        }
    }

private:
    std::vector<Task> _tasks;
};
