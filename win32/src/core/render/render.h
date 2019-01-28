#pragma once

#include "../include.h"

class Mesh;
class Light;
class Camera;
class Shader;

class Render {
public:
	class Matrix {
	public:
		enum ModeType { kPROJECT, kMODEL, };

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
			auto & stack = GetStack(mode);
			if (stack.empty())
			{ stack.push(glm::mat4(1)); }
			else
			{ stack.top() = glm::mat4(1); }
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

		const glm::mat4 & GetP() const
		{
			return Top(ModeType::kPROJECT);
		}

	private:
		std::stack<glm::mat4> & GetStack(ModeType mode)
		{
			return ModeType::kPROJECT == mode ? _project : _model;
		}

		const std::stack<glm::mat4> & GetStack(ModeType mode) const
		{
			return const_cast<Matrix *>(this)->GetStack(mode);
		}

	private:
		std::stack<glm::mat4> _project;
		std::stack<glm::mat4> _model;
	};

    struct CameraInfo {
		size_t mID;
		Camera * mCamera;
        CameraInfo(): mCamera(nullptr), mID(0) { }
        CameraInfo(Camera * camera, size_t id) : mCamera(camera), mID(id) { }
        bool operator ==(size_t id) const { return mID == id; }
        bool operator <(size_t id) const { return mID < id; }
    };

	struct RenderInfo {
		Camera * mCamera;
		Shader * mShader;
		Mesh * mMesh;
	};

    //  ±ä»»ÃüÁî
    struct CommandTransform {
		static void Post(size_t cameraID, const glm::mat4 & mat);
		static void Free(size_t cameraID);
    };

    struct Command {
        size_t mCameraID;
        std::function<void()> mCallFn;
        Command(): mCameraID(0) { }
    };

public:
    Render();
    ~Render();

	Matrix & GetMatrix();

	void Bind(Mesh * mesh);
	void Bind(Shader * shader);
	void Bind(Camera * camera);

	Camera * GetCamera(size_t id);
    void AddCamera(size_t id, Camera * camera);
    void DelCamera(size_t id);

	void BindLight();
	void AddLight(Light * light);
	void DelLight(Light * light);
    
	void RenderVex(GLuint vao, size_t count);
	void RenderIdx(GLuint vao, size_t count);
	void RenderVAO(GLuint vao);
	void RenderMesh();
	void RenderOnce();

	void PostCommand(const Command & command);

private:
	void OnRenderCamera(CameraInfo & camera);
	const glm::mat4 & GetMatrixV() const;
	const glm::mat4 & GetMatrixM() const;
	const glm::mat4 & GetMatrixP() const;
	glm::mat4 GetMatrixMV() const;
	glm::mat4 GetMatrixMVP() const;
	glm::mat3 GetMatrixN() const;

private:
	Matrix _matrix;
	RenderInfo _renderInfo;
	std::vector<Light *> _lights;
    std::vector<Command> _commands;
	std::vector<CameraInfo> _cameraInfos;
};