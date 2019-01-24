#pragma once

#include "asset.h"
#include "mesh.h"
#include "texture.h"
#include "material.h"

class Model: public Asset {
public:
	std::vector<Model *> mChilds;
	std::vector<Mesh *> mMeshs;
	std::vector<Material> mMaterials;
	
	~Model()
	{
		auto delModel = [](Model * model) { delete model; };
		auto delMesh = [](Mesh * mesh) { delete mesh; };
		std::for_each(mChilds.begin(), mChilds.end(), delModel);
		std::for_each(mMeshs.begin(), mMeshs.end(), delMesh);
	}
};
