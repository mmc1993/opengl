#pragma once

#include "res.h"
#include "material.h"
#include "../render/render_type.h"

class Model: public Res {
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
