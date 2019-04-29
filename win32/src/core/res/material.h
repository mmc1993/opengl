#pragma once

#include "res.h"
#include "bitmap.h"
#include "bitmap_cube.h"

class Material {
public:
	Material(): mShininess(32.0f), mTexCube(nullptr)
	{ }
	std::vector<Bitmap *> mDiffuses;
	BitmapCube * mTexCube;			//	立方体贴图
	Bitmap * mSpecular;				//	高光贴图
	Bitmap * mReflect;				//	反射贴图
	Bitmap * mNormal;				//	法线贴图
	Bitmap * mHeight;				//	高度贴图
	float mShininess;				//	光泽度
};