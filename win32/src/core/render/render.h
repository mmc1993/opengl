#pragma once

#include "../include.h"

class Camera;
class Shader;

class Render {
public:
    enum RenderQueueFlag {
        //  背景
        kBACKGROUND,
        //  集合
        kGEOMETRY,
        //  透明
        kOPATCIY,
        //  顶层
        kOVERLAY,
        //  标记最大值
        MAX,
    };

    struct CameraInfo {
        Camera * mCamera;
        size_t mID;

        CameraInfo(): mCamera(nullptr), mID(0)
        { }

        CameraInfo(Camera * camera, size_t id): mCamera(camera), mID(id)
        { }

        bool operator ==(size_t id) const
        {
            return mID == id;
        }

        bool operator !=(size_t id) const
        {
            return mID != id;
        }

        bool operator <(size_t id) const
        {
            return mID < id;
        }

        bool operator >(size_t id) const
        {
            return mID > id;
        }
    };

    struct Command {
        size_t mCameraID;
        Shader * mShader;
        std::function<void()> mCallFn;
    };

public:
    Render();

    ~Render();

    void AddCamera(Camera * camera, size_t id);
    void DelCamera(size_t id);

    void PostCommand(const Command & command);

    void DoRender();

private:
    void RenderCameras();
    void RenderObjects(CameraInfo & camera);

private:
    std::vector<CameraInfo> _cameras;

    std::vector<std::vector<Command>> _renderQueue;
};