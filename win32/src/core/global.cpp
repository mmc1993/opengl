#include "global.h"
#include "event/event.h"
#include "timer/timer.h"
#include "object/object.h"
#include "render/render.h"
#include "res/res_manager.h"
#include "cfg/cfg_cache.h"

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
    , _resManager(nullptr)
    , _cfgCache(nullptr)
{ }

Global::~Global()
{
    Clean();
}

void Global::Start()
{
    ASSERT_LOG(_event == nullptr, "_event Error");
    ASSERT_LOG(_timer == nullptr, "_timer Error");
    ASSERT_LOG(_object == nullptr, "_object Error");
    ASSERT_LOG(_render == nullptr, "_render Error");
    ASSERT_LOG(_resManager == nullptr, "_resManager Error");
    ASSERT_LOG(_cfgCache == nullptr, "_cfgCache Error");

    //  初始化 event
    _event = new Event();

    //  初始化timer
    _timer = new Timer();

    //  初始化object
    _object = new Object();

    //  初始化render
    _render = new Render();

    //  初始化assetCache
    _resManager = new ResManager();

    //  初始化configCache
    _cfgCache = new CfgCache();
    _cfgCache->Init(DIR_CONFIG_ROOT);
}

void Global::Clean()
{
    SafeDelete(_event);
    SafeDelete(_timer);
    SafeDelete(_object);
    SafeDelete(_render);
    SafeDelete(_resManager);
    SafeDelete(_cfgCache);
}
