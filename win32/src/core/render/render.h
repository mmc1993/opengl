#pragma once

#include "../include.h"
#include "render_util.h"
#include "render_enum.h"
#include "render_target.h"

class Render {
public:
    static constexpr uint LIMIT_LIGHT_DIRECT = 2;
    static constexpr uint LIMIT_LIGHT_POINT = 4;
    static constexpr uint LIMIT_LIGHT_SPOT = 4;

    //  对应 _uboLightForward[3]
    enum UBOLightForwardTypeEnum {
        kDIRECT,
        kPOINT,
        kSPOT,
    };

    //  渲染信息
	struct RenderInfo {
        //  记录当前批次顶点数
		uint mVertexCount;
        //  记录当前批次渲染数
		uint mRenderCount;
        //  记录当前Texture基址
        uint mTexBase;

        //  正向渲染数据
        uint mCountUseLightDirect;
        uint mCountUseLightPoint;
        uint mCountUseLightSpot;

        //  当前绑定的pass
        const Pass * mPass;
        //  当前绑定的camera
        const CameraCommand * mCamera;
		RenderInfo()
            : mPass(nullptr)
            , mCamera(nullptr)
            , mVertexCount(0)
			, mRenderCount(0)
            , mTexBase(0) { }
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
	void RenderOnce();
	void PostCommand(const Shader * shader, const RenderCommand & command);
    void PostCommand(const RenderCommand::TypeEnum type, const RenderCommand & command);
	const RenderInfo & GetRenderInfo() const { return _renderInfo; }

private:
    void StartRender();

    //  Bind Function
    bool Bind(const Pass * pass);
    void Bind(const CameraCommand * command);

    //  Post Function
    void Post(const Light * light);
    void Post(const Material & material);
    void Post(const glm::mat4 & transform);
    void Draw(DrawTypeEnum drawType, const Mesh & mesh);

    void ClearCommands();

    //  逐相机渲染
    void SortLightCommands();
    void BakeLightDepthMap();
    void BakeLightDepthMap(Light * light);

    void RenderCamera();
    void RenderForward();
    void RenderDeferred();
	void RenderForwardCommands(const ObjectCommandQueue & commands);
	void RenderDeferredCommands(const ObjectCommandQueue & commands);
    void RenderLightVolume(const LightCommand & command, bool isRenderShadow);

    //  正向渲染相关
    void PackUBOLightForward();
    void BindUBOLightForward();

private:
    RenderTarget    _renderTarget[2];
    MatrixStack     _matrixStack;
	RenderInfo      _renderInfo;

    //	阴影烘培队列
    ObjectCommandQueue _shadowCommands;
    //  方向光队列
    std::array<LightCommandQueue, 3> _lightQueues;
    //  正向渲染队列
    std::array<ObjectCommandQueue, 4> _forwardQueues;
    //  延迟渲染队列
    std::array<ObjectCommandQueue, 4> _deferredQueues;

    //  离屏buffer
    OffSceneBuffer _offSceneBuffer;
    //  正向渲染
    uint _uboLightForward[3];
    //  延迟渲染
    GBuffer _gbuffer;


    //  新队列
    CameraCommandQueue _cameraQueue;
};

