#pragma once

#include "asset.h"
#include "texture.h"

class Material {
public:
	Material(): mShininess(32.0f)
	{ }
	std::vector<Texture> mDiffuses;
	Texture mSpecular;				//	高光贴图
	Texture mReflect;				//	反射贴图
	Texture mNormal;				//	法线贴图
    Texture mHeight;				//	高度贴图
	float mShininess;				//	光泽度
};