#pragma once

#include "asset.h"
#include "texture.h"
#include "bitmap_cube.h"

class Material {
public:
	Material(): mShininess(32.0f), mTexCube(nullptr)
	{ }
	std::vector<Texture> mDiffuses;
	BitmapCube * mTexCube;			//	立方体贴图
	Texture mSpecular;				//	高光贴图
	Texture mReflect;				//	反射贴图
	Texture mNormal;				//	法线贴图
    Texture mHeight;				//	高度贴图
	float mShininess;				//	光泽度
};