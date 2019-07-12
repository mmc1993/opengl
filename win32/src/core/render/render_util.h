#pragma once

#include "../include.h"

//  资源
#include "../raw/gl_mesh.h"
#include "../raw/gl_program.h"
#include "../raw/gl_material.h"
#include "../raw/gl_texture2d.h"

class Light;

class MatrixStack {
public:
	enum ModeType { kPROJ, kVIEW, kMODEL, };

public:
	MatrixStack()
	{ }

	~MatrixStack()
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
		return Top(ModeType::kPROJ);
	}

private:
	std::stack<glm::mat4> & GetStack(ModeType mode) const
	{
		return _matrixs.at((size_t)mode);
	}

private:
	mutable std::array<std::stack<glm::mat4>, 3> _matrixs;
};


//  用于渲染的命令结构
struct RenderCommand {
    enum TypeEnum {
        kMATERIAL,
        kCAMERA,
        kLIGHT,
    };
};

struct MaterialCommand : public RenderCommand {
    MaterialCommand() { }
    const GLMaterial *  mMaterial;
    glm::mat4           mTransform;
    uint                mCameraMask;
    uint                mSubPass;
};

struct CameraCommand : public RenderCommand {
    CameraCommand() { }
    glm::vec4   mViewport;
    glm::mat4   mProj;
    glm::mat4   mView;
    glm::vec3   mPos;
    glm::vec3   mEye;
    float       mN, mF;
    uint        mOrder;
    uint        mMask;

    bool operator<(const CameraCommand & other) const
    {
        return mOrder < other.mOrder;
    }
};

struct LightCommand: public RenderCommand {
    LightCommand() { }
    Light        * mLight;
    GLMesh       * mMesh;
    GLProgram    * mProgram;
    glm::mat4      mTransform;
};

using MaterialCommandQueue = std::vector<MaterialCommand>;
using CameraCommandQueue = std::vector<CameraCommand>;
using LightCommandQueue = std::vector<LightCommand>;
