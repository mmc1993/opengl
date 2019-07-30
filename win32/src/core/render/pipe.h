#pragma once

#include "../include.h"
#include "render_type.h"
#include "render_target.h"

class PipeState {
public:
    PipeState();

    //  光源数据
    uint mLightUBO[3];

    //  渲染时信息
    struct RenderTime {
        //  记录当前批次顶点数
        uint mVertexCount;
        //  记录当前批次渲染数
        uint mDrawCount;
        //  记录当前Texture基址
        uint mTexBase;
        //  当前绑定的Program
        const GLProgram     * mProgram;
        //  当前绑定的Camera
        const CameraCommand * mCamera;
        RenderTime()
            : mProgram(nullptr)
            , mCamera(nullptr)
            , mVertexCount(0)
            , mDrawCount(0)
            , mTexBase(0) 
        { }
    } mRenderTime;

    //  G-Buffer
    struct GBuffer {
        uint mPositionTexture;
        uint mDiffuseTexture;
        uint mNormalTexture;
    } mGBuffer;

    //  阴影贴图
    struct ShadowMap {
        uint mDirectTexture[LIMIT_LIGHT_DIRECT];
        uint mPointTexture[LIMIT_LIGHT_POINT];
        uint mSpotTexture[LIMIT_LIGHT_SPOT];
    } mShadowMap;

    //  后期屏幕
    struct PostScreen {
        uint mColorTexture;
        uint mDepthTexture;
    } mPostScreen;

    //  SSAO
    struct SSAO {
        uint mOcclusionTexture0;    //  一阶段(模糊前)
        uint mOcclusionTexture1;    //  二阶段(模糊后)
    } mSSAO;

    //  渲染树矩阵
    MatrixStack mMatrixStack;

    //  渲染目标
    RenderTarget mRenderTarget[2];

    //  相机渲染队列
    std::vector<CameraCommand> mCameraQueue;
    //  深度队列
    std::vector<MaterialCommand> mDepthQueue;
    //  阴影烘培队列
    std::vector<MaterialCommand> mShadowQueue;
    //  光源类型队列
    std::array<std::vector<LightCommand>, 3> mLightQueues;
    //  正向渲染队列
    std::array<std::vector<MaterialCommand>, 4> mForwardQueues;
    //  延迟渲染队列
    std::array<std::vector<MaterialCommand>, 4> mDeferredQueues;
};

class Renderer;

class Pipe {
public:
    virtual ~Pipe() {}
    virtual void OnAdd(Renderer * renderer, PipeState * state) = 0;
    virtual void OnDel(Renderer * renderer, PipeState * state) = 0;
    virtual void OnUpdate(Renderer * renderer, PipeState * state) = 0;
};