#include "mmc.h"
#include "event/event.h"
#include "timer/timer.h"
#include "object/object.h"
#include "render/render.h"

namespace mmc {
    Event mEvent;
    Timer mTimer;
    Object mRoot;
    Render mRender;
}