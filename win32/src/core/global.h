#pragma once

class Event;
class Timer;
class Object;
class Render;
class Window;
class CfgManager;
class RawManager;

class Global {
public:
    inline void BindWindow(Window * window)
    {
        _window = window;
    }

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

    inline Window & RefWindow()
    {
        return *_window;
    }

    inline CfgManager & RefCfgManager()
    {
        return *_cfgManager;
    }

    inline RawManager & RefRawManager()
    {
        return *_rawManager;
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
    Window * _window;
    CfgManager * _cfgManager;
    RawManager * _rawManager;
};
