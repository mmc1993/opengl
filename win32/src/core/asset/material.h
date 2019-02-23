#pragma once

#include "asset.h"
#include "texture.h"

class Material {
public:
	Material(): mShininess(32.0f)
	{ }
	float mShininess;
	std::vector<Texture> mNormals;
	std::vector<Texture> mDiffuses;
	std::vector<Texture> mSpeculars;
	std::vector<Texture> mReflects;			//	·´ÉäÌùÍ¼
	std::vector<Texture> mParallaxs;		//	ÊÓ²îÌùÍ¼
};