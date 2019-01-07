#pragma once

#include "../include.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"

class Material {
public:
	struct Data {
		std::string mShaderV;
		std::string mShaderF;
		std::string mMeshURL;
		std::string mNormalURL;
		std::string mTextureURL;
		Texture * mTexture;
		Texture * mNormal;
		Shader * mShader;
		Mesh * mMesh;
	};

public:
	Material(Data && data) : _data(std::move(data))
	{
	}

	~Material()
	{
	}

	Data & GetData() { return _data; }

	void OnDraw();
private:
	Data _data;
};