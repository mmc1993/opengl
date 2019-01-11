#pragma once

#include "../include.h"
#include "light.h"

class Mesh;
class Camera;
class Shader;
class Material;

class Render {
public:
	enum GL_UBO_IDX {
		kLIGHT,
	};

	class Matrix {
	public:
		enum ModeType { kPROJECT, kMODELVIEW, };

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

		glm::mat4 GetMVP() const
		{
			return _project.top() * _modelview.top();
		}

		const glm::mat4 & GetMV() const
		{
			return Top(ModeType::kMODELVIEW);
		}

		const glm::mat4 & GetP() const
		{
			return Top(ModeType::kPROJECT);
		}

	private:
		std::stack<glm::mat4> & GetStack(ModeType mode)
		{
			return ModeType::kPROJECT == mode ? _project : _modelview;
		}

		const std::stack<glm::mat4> & GetStack(ModeType mode) const
		{
			return const_cast<Matrix *>(this)->GetStack(mode);
		}

	private:
		std::stack<glm::mat4> _project;
		std::stack<glm::mat4> _modelview;
	};

	class Light {
	public:
		static constexpr size_t MAX_POINT = 32;
		static constexpr size_t MAX_SPOT = 32;

		struct UBO {
			float mAmbient;
			short mSpotNum;
			short mPointNum;
			::LightSpot::Value mSpots[MAX_SPOT];
			::LightPoint::Value mPoints[MAX_POINT];
			UBO()
			{
				memset(this, 0, sizeof(UBO));
			}
		};

	public:
		void SetAmbient(float value);
		void Add(::Light * light);
		void Del(::Light * light);
		void Bind(GLuint shaderID);

		float GetAmbient() const;
		const std::vector<::LightPoint *> & GetPoints() const;
		const std::vector<::LightSpot *> & GetSpots() const;

	private:
		void Update();
		bool CheckCount(::Light * light) const;

	private:
		float _ambient;
		std::vector<::LightPoint *> _points;
		std::vector<::LightSpot *> _spots;
		GLuint _GLID;
		bool _change;
	};

    struct Camera {
		size_t mID;
		::Camera * mCamera;
        Camera(): mCamera(nullptr), mID(0) { }
        Camera(::Camera * camera, size_t id)
			: mCamera(camera), mID(id) { }
        bool operator ==(size_t id) const { return mID == id; }
        bool operator <(size_t id) const { return mID < id; }
    };

	struct RenderInfo {
		Material * mMaterial;
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

	Light & GetLight();
	Matrix & GetMatrix();

	void Bind(Mesh * mesh);
	void Bind(Shader * shader);
	void Bind(Material * material);

    void AddCamera(size_t id, ::Camera * camera);
    void DelCamera(size_t id);
    
	void RenderMesh(size_t count);
	void RenderOnce();

	void PostCommand(const Command & command);

private:
	void OnRenderCamera(Camera & camera);

private:
	Light _light;
	Matrix _matrix;
	RenderInfo _renderInfo;
    std::vector<Camera> _cameras;
    std::vector<Command> _commands;
};