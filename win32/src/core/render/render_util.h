#pragma once

#include "../include.h"

class Mesh;
class Pass;
class Light;
class Camera;
class Shader;
class Bitmap;
class Material;
class BitmapCube;

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
        kOBJECT,
        kLIGHT,
    };
    RenderCommand(TypeEnum type): mType(type) 
    { }
    TypeEnum mType;
};

struct CameraCommand : public RenderCommand {
    CameraCommand(): RenderCommand(kCAMERA)
    { }
    glm::vec4 mViewport;
    glm::mat4 mProj;
    glm::mat4 mView;
    glm::vec3 mPos;
    glm::vec3 mEye;
    uint mOrder;
    uint mMask;
};

struct ObjectCommand: public RenderCommand {
    ObjectCommand(): RenderCommand(kOBJECT)
    { }
    const Pass      * mPass;            //  绑定的Pass
    const Mesh      * mMeshs;           //  绑定的Mesh
    const Material  * mMaterials;       //  绑定的材质(材质与网格数量必须一致)
    uint mMeshNum;                      //  绑定的网格数量
    uint mCameraFlag;                   //  绑定的相机标识
    glm::mat4 mTransform;               //  绑定的变换矩阵
};

struct LightCommand: public RenderCommand {
    LightCommand(): RenderCommand(kLIGHT)
    { }
    Light        * mLight;
    const Mesh   * mMesh;
    const Shader * mShader;
    glm::mat4 mTransform;
};

using CameraCommandQueue = std::vector<CameraCommand>;
using ObjectCommandQueue = std::vector<ObjectCommand>;
using LightCommandQueue = std::vector<LightCommand>;
