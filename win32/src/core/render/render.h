#pragma once

#include "../include.h"

class Light;
class Camera;
class Shader;
class Bitmap;
class BitmapCube;
class Texture;

class Render {
public:
	class Matrix {
	public:
		enum ModeType { kPROJECT, kVIEW, kMODEL, };

	public:
		Matrix()
		{ }

		~Matrix()
		{ }

		void Pop(ModeType mode)
		{
			GetStack(mode).pop();
		}

		void Push(ModeType mode)
		{
			GetStack(mode).push(GetStack(mode).top());
		}

		void Identity(ModeType mode)
		{
			GetStack(mode).push(glm::mat4(1));
		}

		void Mul(ModeType mode, const glm::mat4 & mat)
		{
			GetStack(mode).top() *= mat;
		}

		const glm::mat4 & Top(ModeType mode) const
		{
			return GetStack(mode).top();
		}

		const glm::mat4 & GetM() const
		{
			return Top(ModeType::kMODEL);
		}

		const glm::mat4 & GetV() const
		{
			return Top(ModeType::kVIEW);
		}

		const glm::mat4 & GetP() const
		{
			return Top(ModeType::kPROJECT);
		}

	private:
		std::stack<glm::mat4> & GetStack(ModeType mode)
		{
			return _matrixs.at((size_t)mode);
		}

		const std::stack<glm::mat4> & GetStack(ModeType mode) const
		{
			return const_cast<Matrix *>(this)->GetStack(mode);
		}

	private:
		std::array<std::stack<glm::mat4>, 3> _matrixs;
	};

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
		RenderInfo()
			: mTexCount(0)
			, mCamera(nullptr)
			, mShader(nullptr)
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

	Matrix & GetMatrix();

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

private:
	glm::mat4 GetMatrixMVP() const;
	glm::mat4 GetMatrixMV() const;
	glm::mat3 GetMatrixN() const;
	void RenderVAO(GLuint vao);

private:
	Matrix _matrix;
	RenderInfo _renderInfo;
	std::vector<Light *> _lights;
    std::vector<Command> _commands;
	std::vector<CameraInfo> _cameraInfos;
};