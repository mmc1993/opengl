#pragma once

#include "asset.h"
#include "texture.h"

class Material {
public:
	Material(): mShinines(32.0f)
	{ }
	std::vector<Texture> mDiffuses;
	Texture mParallax;				//	视差贴图
	Texture mSpecular;				//	高光贴图
	Texture mReflect;				//	反射贴图
	Texture mNormal;				//	法线贴图
	float mShinines;				//	光泽度
};