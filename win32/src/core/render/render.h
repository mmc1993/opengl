#pragma once

#include "../include.h"
#include "render_type.h"

class Light;
class Camera;
class Shader;
class Bitmap;
class BitmapCube;
class Texture;

class Render {
public:
    struct CameraInfo {
		enum Flag {
			kFlag0 = 0x1,	kFlag1 = 0x2,	kFlag2 = 0x4,	kFlag3 = 0x8,
			kFlag4 = 0x10,	kFlag5 = 0x20,	kFlag6 = 0x30,	kFlag7 = 0x40,
		};

		size_t mFlag;
		size_t mOrder;
		Camera * mCamera;
        CameraInfo(): mCamera(nullptr), mOrder(0), mFlag(0) { }
        CameraInfo(Camera * camera, size_t flag, size_t order) 
			: mCamera(camera), mFlag(flag), mOrder(order) { }
    };

	struct RenderInfo {
		size_t mTexCount;
		Camera * mCamera;
		Shader * mShader;
		size_t mVertexCount;
		size_t mRenderCount;
		RenderInfo()
			: mTexCount(0)
			, mCamera(nullptr)
			, mShader(nullptr)
			, mVertexCount(0)
			, mRenderCount(0)
		{ }
	};

    //  ±‰ªª√¸¡Ó
    struct CommandTransform {
		static void Post(size_t cameraFlag, const glm::mat4 & mat);
		static void Free(size_t cameraFlag);
    };

    struct Command {
		size_t mCameraFlag;
        std::function<void()> mCallFn;
        Command(): mCameraFlag(0) { }
    };

public:
    Render();
    ~Render();

	RenderMatrix & GetMatrix();

	void Bind(Shader * shader);
	void Bind(Camera * camera);

    void AddCamera(Camera * camera, size_t flag, size_t order = (size_t)-1);
	Camera * GetCamera(size_t order);
	void DelCamera(Camera * camera);
	void DelCamera(size_t order);

	void BindLight();
	void AddLight(Light * light);
	void DelLight(Light * light);
    
	void RenderVexInst(GLuint vao, size_t count, size_t instanceCount);
	void RenderIdxInst(GLuint vao, size_t count, size_t instanceCount);
	void RenderVex(GLuint vao, size_t count);
	void RenderIdx(GLuint vao, size_t count);
	void OnRenderCamera(CameraInfo * camera);
	void RenderOnce();

	void BindTexture(const std::string & key, const Texture & val);
	void BindTexture(const std::string & key, const Bitmap * val);
	void BindTexture(const std::string & key, const BitmapCube * val);

	void PostCommand(const Command & command);

	const RenderInfo & GetRenderInfo() const
	{
		return _renderInfo;
	}

private:
	glm::mat4 GetMatrixMVP() const;
	glm::mat4 GetMatrixMV() const;
	glm::mat3 GetMatrixN() const;
	void RenderVAO(GLuint vao);

private:
    RenderMatrix _matrix;
	RenderInfo _renderInfo;
	std::vector<Light *> _lights;
    std::vector<Command> _commands;
	std::vector<CameraInfo> _cameraInfos;
};