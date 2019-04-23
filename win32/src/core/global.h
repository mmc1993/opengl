#pragma once

class Event;
class Timer;
class Object;
class Render;
class AssetCache;
class ConfigCache;

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

    inline AssetCache & RefAssetCache()
    {
        return *_assetCache;
    }

    inline ConfigCache & RefConfigCache()
    {
        return *_configCache;
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
    AssetCache * _assetCache;
    ConfigCache * _configCache;
};
