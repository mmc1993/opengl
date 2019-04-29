#include "global.h"
#include "event/event.h"
#include "timer/timer.h"
#include "object/object.h"
#include "render/render.h"
#include "asset/asset_cache.h"
#include "config/config_cache.h"

//  配置根目录
constexpr auto DIR_CONFIG_ROOT = "res/config";

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
    //  初始化 event
    _event = new Event();

    //  初始化timer
    _timer = new Timer();

    //  初始化object
    _object = new Object();

    //  初始化render
    _render = new Render();

    //  初始化assetCache
    _assetCache = new ResCache();

    //  初始化configCache
    _configCache = new ConfigCache();
    _configCache->Init(DIR_CONFIG_ROOT);
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
