#pragma once

#include "../include.h"
#include "render_type.h"
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
		uint mVertexCount;
		uint mRenderCount;
        uint mTexBase;

        //  当前正向渲染使用的光源

        //  当前绑定的pass
        const RenderPass * mPass;
        //  当前绑定的camera
        const CameraInfo * mCamera;
		RenderInfo()
            : mPass(nullptr)
            , mCamera(nullptr)
            , mVertexCount(0)
			, mRenderCount(0)
            , mTexBase(0) { }
	};


public:
    Render();
    ~Render();

	RenderMatrix & GetMatrix();

    //  相机
    void AddCamera(Camera * camera, size_t flag, size_t order = ~0);
	Camera *GetCamera(size_t order);
	void DelCamera(Camera * camera);
	void DelCamera(size_t order);

    //  光源
	void AddLight(Light * light);
	void DelLight(Light * light);
    
    //  渲染
	void RenderOnce();

	void PostCommand(const Shader * shader, const RenderCommand & command);

	const RenderInfo & GetRenderInfo() const { return _renderInfo; }

private:
    void Bind(const Light * light);
    bool Bind(const RenderPass * pass);
    void Bind(const CameraInfo * camera);
    void Bind(const Material * material);
    void Bind(const RenderCommand & command);

    //	执行绘制命令
    void Draw(DrawTypeEnum drawType, const RenderMesh & mesh);

    void ClearCommands();

    //  生成ShadowMap
	void RenderShadow(Light * light);

    //  逐相机渲染
    void RenderCamera();
    void RenderForward();
    void RenderDeferred();
	void RenderForwardCommands(const RenderQueue & commands);
	void RenderDeferredCommands(Light * light, const RenderQueue & commands);

    //  正向渲染光源相关
    void InitUBOLightForward();
    void PackUBOLightForward();
    void BindUBOLightForward();

private:
    RenderMatrix    _matrix;
    RenderTarget    _shadowRT;
	RenderInfo      _renderInfo;

	std::vector<Light *>    _lights;
    std::vector<CameraInfo> _cameraInfos;

	//	渲染队列, 阴影不需要区分队列类型
    RenderQueue _shadowCommands;
    std::array<RenderQueue, 4> _forwardCommands;
    std::array<RenderQueue, 4> _deferredCommands;

    //  光源UBO
    uint _uboLightForward[3];
};

