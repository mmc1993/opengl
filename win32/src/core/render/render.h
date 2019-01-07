#pragma once

#include "../include.h"

class Camera;
class Shader;

class Render {
public:
	//  ‰÷»æ∂”¡–
	enum QueueType {
		//  ±≥æ∞
		kBACKGROUND,
		//  º∏∫Œ
		kGEOMETRY,
		//  Õ∏√˜
		kOPATCIY,
		//  ∂•≤„
		kOVERLAY,
		MAX,
	};

	enum CommandType {
		//  ‰÷»æ
		kRENDER,
		//  æÿ’Û±‰ªØ
		kTRANSFORM,
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
			GetStack(mode).top() = glm::identity<glm::mat4>();
		}

		void Mul(ModeType mode, const glm::mat4 & mat)
		{
			GetStack(mode).top() *= mat;
		}

		const glm::mat4 & Top(ModeType mode) const
		{
			return GetStack(mode).top();
		}

	private:
		std::stack<glm::mat4> & GetStack(ModeType mode)
		{
			return ModeType::kPROJECT == mode
				? _project : _modelview;
		}

		const std::stack<glm::mat4> & GetStack(ModeType mode) const
		{
			return const_cast<Matrix *>(this)->GetStack(mode);
		}

	private:
		std::stack<glm::mat4> _project;
		std::stack<glm::mat4> _modelview;
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

    //  ±‰ªª√¸¡Ó
    struct CommandTransform {
        const bool mIsPush;
        const glm::mat4 * mMatrix;

        CommandTransform(
            const bool ispush, 
            const glm::mat4 * mat4)
            : mIsPush(ispush), mMatrix(mat4)
        { }

        void operator ()();
    };

    //  ‰÷»æ√¸¡Ó
    struct CommandRender {
        //void operator ()();
    };

    struct Command {
        size_t mCameraID;
        Shader * mShader;
        CommandType mType;
        std::function<void()> mCallFn;

        Command(): mCameraID(0), mShader(nullptr)
        { }
    };

public:
    Render();
    ~Render();

    void AddCamera(::Camera * camera, size_t id);
    void DelCamera(size_t id);

	Matrix & GetMatrix()
	{
		return _matrix;
	}

    void PostCommand(const Command & command);

    void DoRender();

private:
    void RenderObjects(Camera & camera);

private:
	Matrix _matrix;

    std::vector<Camera> _cameras;

    std::vector<std::vector<Command>> _renderQueue;
};