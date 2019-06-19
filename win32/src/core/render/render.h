#pragma once

#include "../include.h"
#include "render_util.h"
#include "render_enum.h"
#include "render_target.h"

class Render {
public:
    static constexpr uint LIMIT_LIGHT_DIRECT = 1;
    static constexpr uint LIMIT_LIGHT_POINT = 2;
    static constexpr uint LIMIT_LIGHT_SPOT = 3;

    //  渲染状态
    struct RenderState {
        //  记录当前批次顶点数
        uint mVertexCount;
        //  记录当前批次渲染数
        uint mRenderCount;
        //  记录当前Texture基址
        uint mTexBase;

        //  当前绑定的Program
        const GLProgram     * mProgram;
        //  当前绑定的Camera
        const CameraCommand * mCamera;
        RenderState()
            : mProgram(nullptr)
            , mCamera(nullptr)
            , mVertexCount(0)
            , mRenderCount(0)
            , mTexBase(0) { }
    };

    struct TextureBufferSet {
        //  G-Buffer
        struct GBuffer {
            uint mPositionTexture;
            uint mSpecularTexture;
            uint mDiffuseTexture;
            uint mNormalTexture;
            uint mDepthBuffer;
        } mGBuffer;

        //  阴影贴图
        struct ShadowMap {
            uint mDirectTexture[LIMIT_LIGHT_DIRECT];
            uint mPointTexture[LIMIT_LIGHT_POINT];
            uint mSpotTexture[LIMIT_LIGHT_SPOT];
        } mShadowMap;

        //  离屏缓存
        struct OffScreen {
            uint mColorTexture;
            uint mDepthTexture;
        } mOffScreen;

        TextureBufferSet()
        {
            memset(this, 0, sizeof(TextureBufferSet));
        }
    };

    struct GBuffer {
        uint mPositionTexture;
        uint mSpecularTexture;
        uint mDiffuseTexture;
        uint mNormalTexture;
        uint mDepthBuffer;
        GBuffer()
            : mPositionTexture(0)
            , mSpecularTexture(0)
            , mDiffuseTexture(0)
            , mNormalTexture(0)
            , mDepthBuffer(0)
        { }
    };

    struct OffSceneBuffer {
        uint mColorTexture;
        uint mDepthTexture;
        OffSceneBuffer()
            : mColorTexture(0)
            , mDepthTexture(0)
        { }
    };

public:
    Render();
    ~Render();

	MatrixStack & GetMatrixStack();
    //  渲染
	void Once();
    //  渲染命令入口
    void Post(const RenderCommand::TypeEnum type, const RenderCommand & command);

    const RenderState & GetRenderState() const { return _renderState; }

private:
    void StartRender();

    //  Bind 系函数.
    //      该系列函数完成数据提交同时影响渲染器内部状态
    void Bind(const CameraCommand * command);
    bool Bind(const GLProgram     * program, uint pass);

    //  Post 系函数.
    //      该系函数完成数据提交, 但不修改渲染器内部状态
    void Post(const Light *light);
    void Post(const GLMaterial * material);
    void Post(const glm::mat4 & transform);
    void Post(DrawTypeEnum drawType, const GLMesh * mesh);

    void ClearCommands();

    //  逐相机渲染
    void SortLightCommands();
    void BakeLightDepthMap();
    void BakeLightDepthMap(Light * light, uint shadow);

    void RenderCamera();
    void RenderForward();
    void RenderDeferred();
	void RenderForwardCommands(const MaterialCommandQueue & commands);
	void RenderDeferredCommands(const MaterialCommandQueue & commands);
    void RenderDeferredLightVolume(const LightCommand & command, uint shadow);

    //  正向渲染相关
    void PackUBOLightForward();
    void BindUBOLightForward();

private:
    TextureBufferSet    _textureBufferSet;
    RenderTarget        _renderTarget[2];
    MatrixStack         _matrixStack;

    //  离屏buffer
    OffSceneBuffer _offSceneBuffer;
    //  延迟渲染
    GBuffer _bufferG;

    //  状态
    RenderState _renderState;
    //  相机渲染队列
    CameraCommandQueue _cameraQueue;
    //  阴影烘培队列
    MaterialCommandQueue _shadowQueue;
    //  光源类型队列
    std::array<LightCommandQueue, 3> _lightQueues;
    //  正向渲染队列
    std::array<MaterialCommandQueue, 4> _forwardQueues;
    //  延迟渲染队列
    std::array<MaterialCommandQueue, 4> _deferredQueues;

    //  深度贴图
    uint _shadowMapDirect[LIMIT_LIGHT_DIRECT];
    uint _shadowMapPoint[LIMIT_LIGHT_POINT];
    uint _shadowMapSpot[LIMIT_LIGHT_SPOT];
    //  光源数据
    uint _lightForwardUBO[3];
};

