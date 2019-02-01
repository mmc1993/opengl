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
		size_t mID;
		size_t mIdx;
		Camera * mCamera;
        CameraInfo(): mCamera(nullptr), mID(0), mIdx(0) { }
        CameraInfo(Camera * camera, size_t idx, size_t id) 
			: mCamera(camera), mIdx(idx), mID(id) { }
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
		static void Post(size_t cameraIdx, const glm::mat4 & mat);
		static void Free(size_t cameraIdx);
    };

    struct Command {
        size_t mCameraIdx;
        std::function<void()> mCallFn;
        Command(): mCameraIdx(0) { }
    };

public:
    Render();
    ~Render();

	Matrix & GetMatrix();

	void Bind(Shader * shader);
	void Bind(Camera * camera);

	Camera * GetCamera(size_t id);
    void AddCamera(size_t idx, Camera * camera, size_t id);
	void DelCamera(size_t idx);
	void DelCamera(Camera * camera);

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