#pragma once

#include "../include.h"

class EventDispatcher {
private:
    static size_t s_coundID;

public:
    using func_t = std::function<void(const std::any &)>;

    struct Event {
        size_t mID;
        func_t mFunc;
        Event(size_t id, const func_t & func): mID(id), mFunc(func)
        { }

        bool operator ==(size_t id) const
        {
            return mID == id;
        }
    };

public:
    EventDispatcher()
    { }

    ~EventDispatcher()
    { }

    size_t Add(size_t type, const func_t & func)
    {
        auto insert = _events.insert(std::make_pair(type, std::vector<Event>()));
        insert.first->second.emplace_back(EventDispatcher::s_coundID, func);
        return EventDispatcher::s_coundID++;
    }

    void Del(size_t id)
    {
        for (auto & events : _events)
        {
            auto it = std::find(events.second.begin(),
                                events.second.end(), id);
            if (it != events.second.end())
            {
                events.second.erase(it);
            }
        }
    }

    void Post(size_t type, std::any param)
    {
        auto it = _events.find(type);
        if (it != _events.end())
        {
            for (auto & e : it->second)
            {
                e.mFunc(param);
            }
        }
    }

private:
    std::map<size_t, std::vector<Event>> _events;
};

class EventMouse {
public:
    struct Param {
        int btn;
        int act;
        float x;
        float y;
        int stat;
    };

    enum {
        kBUTTON,
        kMOVEED,
    };
};

class EventKey {
public:
    struct Param {
        int key;
        int act;
        int stat;
    };

    enum {
        kKEY
    };
};
