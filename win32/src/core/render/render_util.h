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
    enum Type {
        kOBJECT,
        kLIGHT,
    };
    RenderCommand(Type type): mType(type) 
    { }
    Type mType;
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
    Light * mLight;
    const Pass * mPass;
    const Mesh * mMesh;
    glm::mat4 mTransform;
};

using ObjectCommandQueue = std::vector<ObjectCommand>;
using LightCommandQueue = std::vector<LightCommand>;
