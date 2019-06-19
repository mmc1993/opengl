#include "global.h"
#include "event/event.h"
#include "timer/timer.h"
#include "object/object.h"
#include "render/render.h"
#include "cfg/cfg_manager.h"
#include "raw/raw_manager.h"

//  ÅäÖÃ¸ùÄ¿Â¼
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
    , _cfgManager(nullptr)
    , _rawManager(nullptr)
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
    ASSERT_LOG(_cfgManager == nullptr, "_cfgManager Error");
    ASSERT_LOG(_rawManager == nullptr, "_rawManager Error");

    _event = new Event();

    _timer = new Timer();

    _object = new Object();

    _render = new Render();

    _cfgManager = new CfgManager();
    _cfgManager->Init(DIR_CONFIG_ROOT);

    _rawManager = new RawManager();
}

void Global::Clean()
{
    SafeDelete(_event);
    SafeDelete(_timer);
    SafeDelete(_object);
    SafeDelete(_render);
    SafeDelete(_cfgManager);
    SafeDelete(_rawManager);
}
