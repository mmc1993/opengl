#pragma once

class Event;
class Timer;
class Object;
class Render;
class AssetCache;
class ConfigCache;

namespace mmc {
    extern Event mEvent;
    extern Timer mTimer;
    extern Object mRoot;
    extern Render mRender;
	extern AssetCache mAssetCache;
    extern ConfigCache mConfigCache;
}
