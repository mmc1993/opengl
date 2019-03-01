#pragma once

//#include "event/event.h"
//#include "timer/timer.h"
//#include "object/object.h"
//#include "render/render.h"

class Event;
class Timer;
class Object;
class Render;
class AssetCache;

namespace mmc {
    extern Event mEvent;
    extern Timer mTimer;
    extern Object mRoot;
    extern Render mRender;
	extern AssetCache mAssetCore;
}
