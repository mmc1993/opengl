#pragma once

class Event;
class Timer;
class Object;
class Window;
class Renderer;
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

    inline Window & RefWindow()
    {
        return *_window;
    }

    inline Renderer & RefRenderer()
    {
        return *_renderer;
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
    Window * _window;
    Renderer * _renderer;
    CfgManager * _cfgManager;
    RawManager * _rawManager;
};
