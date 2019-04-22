#pragma once

#include "../include.h"
#include "../tools/time_tool.h"

class Timer {
private:
    static size_t s_countID;

public:
    struct Task {
        size_t mID;
		float mTime;
		std::function<void()> mCall;
        
		Task()
        { }

        Task(size_t id, 
			 const float time,
             const std::function<void()> & call)
            : mID(id), mTime(time), mCall(call)
        { }

        bool operator ==(size_t id) const
        {
            return mID == id;
        }

        bool operator <(const Timer::Task & other) const
        {
            return mTime > other.mTime;
        }

        bool operator >(const Timer::Task & other) const
        {
            return mTime < other.mTime;
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
    
    void Update(const float time)
    {
        while (!_tasks.empty() && _tasks.front().mTime < time)
        {
			auto fn = _tasks.front().mCall;
            std::pop_heap(
                std::begin(_tasks), 
                std::end(_tasks));
            _tasks.pop_back();
			fn();
        }
    }

    size_t Add(float time, const std::function<void()> & func)
    {
		_tasks.emplace_back(Timer::s_countID, time_tool::Now(time), func);
        std::push_heap(_tasks.begin(), _tasks.end());
        return Timer::s_countID++;
    }

    void Del(size_t id)
    {
        auto it = std::find(std::begin(_tasks), std::end(_tasks), id);
        if (it != std::end(_tasks))
        {
            _tasks.erase(it);
            std::make_heap(std::begin(_tasks), std::end(_tasks));
        }
    }

private:
    std::vector<Task> _tasks;
};
