#pragma once

#include "../include.h"
#include "render_util.h"
#include "render_enum.h"
#include "render_target.h"

class Render {
public:
    //  正向渲染光源数限制
    static constexpr uint LIMIT_FORWARD_LIGHT_DIRECT = 2;
    static constexpr uint LIMIT_FORWARD_LIGHT_POINT = 4;
    static constexpr uint LIMIT_FORWARD_LIGHT_SPOT = 4;

    //  对应 _uboLightForward[3]
    enum UBOLightForwardTypeEnum {
        kDIRECT,
        kPOINT,
        kSPOT,
    };

    struct CameraInfo {
		enum Flag {
			kFLAG0 = 0x1,	kFLAG1 = 0x2,	kFLAG2 = 0x4,	kFLAG3 = 0x8,
			kFLAG4 = 0x10,	kFLAG5 = 0x20,	kFLAG6 = 0x30,	kFLAG7 = 0x40,
		};

		size_t mFlag;
		size_t mOrder;
		Camera * mCamera;
        CameraInfo(): mCamera(nullptr), mOrder(0), mFlag(0) { }
        CameraInfo(Camera * camera, size_t flag, size_t order) 
			: mCamera(camera), mFlag(flag), mOrder(order) { }
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
        uint mCountForwardLightDirect;
        uint mCountForwardLightPoint;
        uint mCountForwardLightSpot;

        //  当前绑定的pass
        const Pass * mPass;
        //  当前绑定的camera
        const CameraInfo * mCamera;
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
        uint mDepthTexture;
        GBuffer()
            : mPositionTexture(0)
            , mSpecularTexture(0)
            , mDiffuseTexture(0)
            , mNormalTexture(0)
            , mDepthTexture(0)
        { }
    };

public:
    Render();
    ~Render();

	MatrixStack & GetMatrixStack();

    //  相机
    void AddCamera(Camera * camera, size_t flag, size_t order = ~0);
	Camera *GetCamera(size_t order);
	void DelCamera(Camera * camera);
	void DelCamera(size_t order);

    //  渲染
	void RenderOnce();
	void PostCommand(const Shader * shader, const RenderCommand & command);
	const RenderInfo & GetRenderInfo() const { return _renderInfo; }

private:
    //  Bind Function
    bool Bind(const Pass * pass);
    void Bind(const CameraInfo * camera);

    //  Post Function
    void Post(const Light * light);
    void Post(const Material & material);
    void PostMatrix(const glm::mat4 & transform);

    //	执行绘制命令
    void Draw(DrawTypeEnum drawType, const Mesh & mesh);

    void ClearCommands();

    //  生成ShadowMap
	void RenderShadow(Light * light);

    //  逐相机渲染
    void RenderCamera();
    void RenderForward();
    void RenderDeferred();
	void RenderForwardCommands(const ObjectCommandQueue & commands);
	void RenderDeferredCommands(const ObjectCommandQueue & commands);

    //  正向渲染相关
    void InitUBOLightForward();
    void PackUBOLightForward();
    void BindUBOLightForward();

    //  延迟渲染相关
    void InitGBuffer();
    void FillGBuffer();

private:
    RenderTarget    _renderTarget;
    MatrixStack     _matrixStack;
	RenderInfo      _renderInfo;

    //  相机列表
    std::vector<CameraInfo> _cameraInfos;
    //  光源队列
    LightCommandQueue _lightCommands;
    //	阴影烘培队列
    ObjectCommandQueue _shadowCommands;
    //  正向渲染队列
    std::array<ObjectCommandQueue, 4> _forwardCommands;
    //  延迟渲染队列
    std::array<ObjectCommandQueue, 4> _deferredCommands;

    //  正向渲染
    uint _uboLightForward[3];
    //  延迟渲染
    GBuffer _gbuffer;
};

