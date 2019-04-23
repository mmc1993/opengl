#include "global.h"
#include "event/event.h"
#include "timer/timer.h"
#include "object/object.h"
#include "render/render.h"
#include "asset/asset_cache.h"
#include "config/config_cache.h"

template <class T>
void SafeDelete(T & ptr)
{
    delete ptr;
    ptr = nullptr;
}

Global::Global()
    : _event(nullptr)
    , _timer(nullptr)
    , _object(nullptr)
    , _render(nullptr)
    , _assetCache(nullptr)
    , _configCache(nullptr)
{ }

Global::~Global()
{
    Clean();
}

void Global::Start()
{
    assert(_event == nullptr 
        && _timer == nullptr 
        && _object == nullptr 
        && _render == nullptr 
        && _assetCache == nullptr
        && _configCache == nullptr);
    _event = new Event();
    _timer = new Timer();
    _object = new Object();
    _render = new Render();
    _assetCache = new AssetCache();
    _configCache = new ConfigCache();
}

void Global::Clean()
{
    SafeDelete(_event);
    SafeDelete(_timer);
    SafeDelete(_object);
    SafeDelete(_render);
    SafeDelete(_assetCache);
    SafeDelete(_configCache);
}
