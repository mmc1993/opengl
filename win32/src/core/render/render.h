#pragma once

#include "../include.h"
#include "render_type.h"
#include "render_target.h"

class Light;

class Render {
public:
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

    struct BufferSet {
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
        
        //  光源数据
        uint mLightUBO[3];
        
        BufferSet()
        {
            memset(this, 0, sizeof(BufferSet));
        }
    };

public:
    Render();
    ~Render();

	MatrixStack & GetMatrixStack();
    //  渲染
	void Once();
    //  渲染命令入口
    void Post(const CommandEnum & type,
              const RenderCommand & command);

    const RenderState & GetRenderState() const { return _renderState; }

private:
    //  初始化渲染数据
    void InitRender();

    //  清理所有命令
    void ClearCommands();

    //  Bind 系函数.
    //      该系列函数完成数据提交同时影响渲染器内部状态
    void Bind(const CameraCommand * command);
    bool Bind(const GLProgram     * program, uint pass);

    //  Post 系函数.
    //      该系函数完成数据提交, 但不修改渲染器内部状态
    void Post(const LightCommand &command);
    void Post(const GLMaterial * material);
    void Post(const glm::mat4 & transform);
    void Post(DrawTypeEnum drawType, const GLMesh * mesh);

    //  根据距离排序光源
    void SortLightCommands();
    //  烘培阴影
    void BakeLightDepthMap();
    void BakeLightDepthMap(Light * light, uint shadow);

    //  逐相机渲染
    void RenderCamera();
    //  渲染GBuffer
    void RenderGBuffer();
    //  渲染SSAO
    void RenderSSAO();
    //  延迟渲染
    void RenderDeferred();
    //  正向渲染
    void RenderForward();
    //  光体渲染
    void RenderLightVolume(const LightCommand & command, uint shadow);

    //  正向渲染相关
    void PackUBOLightForward();
    void BindUBOLightForward();

private:
    //  0 随意使用, 1 离屏输出
    RenderTarget _target[2];
    BufferSet    _bufferSet;
    MatrixStack  _matrixStack;
    GLMesh     * _screenQuad;
    GLProgram  * _ssaoProgram;

    //  状态
    RenderState _renderState;
    //  相机渲染队列
    std::vector<CameraCommand> _cameraQueue;
    //  深度队列
    std::vector<MaterialCommand> _depthQueue;
    //  阴影烘培队列
    std::vector<MaterialCommand> _shadowQueue;
    //  光源类型队列
    std::array<std::vector<LightCommand>, 3> _lightQueues;
    //  正向渲染队列
    std::array<std::vector<MaterialCommand>, 4> _forwardQueues;
    //  延迟渲染队列
    std::array<std::vector<MaterialCommand>, 4> _deferredQueues;
};

