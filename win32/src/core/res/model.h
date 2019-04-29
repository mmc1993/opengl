#pragma once

#include "res.h"
#include "mesh.h"
#include "material.h"

class Model: public Res {
public:
	std::vector<Model *> mChilds;
	std::vector<Material> mMates;
	std::vector<Mesh> mMeshs;
	
	~Model()
	{
		std::for_each(mMeshs.begin(), mMeshs.end(), Mesh::Delete);
		for (auto child : mChilds) { delete child; }
	}
};
