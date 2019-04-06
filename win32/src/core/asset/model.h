#pragma once

#include "asset.h"
#include "texture.h"
#include "material.h"
#include "../render/render_type.h"

class Model: public Asset {
public:
	std::vector<Model *> mChilds;
	std::vector<Material> mMates;
	std::vector<RenderMesh> mMeshs;
	
	~Model()
	{
		std::for_each(mMeshs.begin(), mMeshs.end(), RenderMesh::Delete);
		for (auto child : mChilds) { delete child; }
	}
};
