#include "global.h"
#include "event/event.h"
#include "timer/timer.h"
#include "object/object.h"
#include "render/renderer.h"
#include "cfg/cfg_manager.h"
#include "raw/raw_manager.h"

Global::Global()
    : _event(nullptr)
    , _timer(nullptr)
    , _object(nullptr)
    , _renderer(nullptr)
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
    ASSERT_LOG(_renderer == nullptr, "_render Error");
    ASSERT_LOG(_cfgManager == nullptr, "_cfgManager Error");
    ASSERT_LOG(_rawManager == nullptr, "_rawManager Error");

    _event = new Event();

    _timer = new Timer();

    _object = new Object();

    _renderer = new Renderer();

    _cfgManager = new CfgManager();

    _rawManager = new RawManager();
}

void Global::Clean()
{
    _window = nullptr;
    SAFE_DELETE(_object);
    SAFE_DELETE(_event);
    SAFE_DELETE(_timer);
    SAFE_DELETE(_renderer);
    SAFE_DELETE(_cfgManager);
    SAFE_DELETE(_rawManager);
    
}
