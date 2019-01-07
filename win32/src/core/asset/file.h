#pragma once

#include "../include.h"
#include "material.h"
#include "mesh.h"

class File {
public:
	static std::vector<Mesh::Vertex> && LoadMesh(const std::string & fname);
	static Material::Data && LoadMaterial(const std::string & fname);
private:
	static std::string_view FindSubStrUntil(
		const std::string_view & str, 
		const std::string_view & end, 
		size_t pos = 0);
};