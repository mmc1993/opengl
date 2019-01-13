#pragma once

#include "../include.h"
#include "asset.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"

class Material: public Asset {
public:
	struct Data {
		glm::vec3 mAmbient;
		glm::vec3 mDiffuse;
		glm::vec3 mSpecular;
		float mShininess;
	};

public:
	Material(const Data & data) : mData(data)
	{ }

	~Material()
	{ }

	Data mData;
};