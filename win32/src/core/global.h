#pragma once

class Event;
class Timer;
class Object;
class Render;
class ResManager;
class CfgManager;

class Global {
public:
    inline Event & RefEvent()
    {
        return *_event;
    }

    inline Timer & RefTimer()
    {
        return *_timer;
    }

    inline Object & RefObject()
    {
        return *_object;
    }

    inline Render & RefRender()
    {
        return *_render;
    }

    inline ResManager & RefResManager()
    {
        return *_resManager;
    }

    inline CfgManager & RefCfgManager()
    {
        return *_cfgManager;
    }

    Global();
    ~Global();

    void Start();
    void Clean();

    static Global & Ref()
    {
        static Global global;
        return global;
    }

private:
    Event * _event;
    Timer * _timer;
    Object * _object;
    Render * _render;
    ResManager * _resManager;
    CfgManager * _cfgManager;
};
