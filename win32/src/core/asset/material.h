#pragma once

#include "asset.h"
#include "texture.h"

class Material: public Asset {
public:
	struct Data {
		Texture mDiffuse;
		Texture mSpecular;
		float mShininess;
	};

public:
	Material(const Data & data) : mData(data)
	{ }

	~Material()
	{ }

	Data mData;
};