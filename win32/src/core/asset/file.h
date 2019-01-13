#pragma once

#include "../include.h"
#include "material.h"
#include "texture.h"
#include "bitmap.h"
#include "shader.h"
#include "mesh.h"

class File {
public:
	static Mesh * LoadMesh(const std::string & url);
	static Shader * LoadShader(const std::string & url);
	static Bitmap * LoadBitmap(const std::string & url, int format);
	static Texture LoadTexture(const std::string & url, int format);
	static Material * LoadMaterial(const std::string & url);
private:
	static std::string_view FindSubStrUntil(
		const std::string_view & str, 
		const std::string_view & end, 
		size_t pos = 0);
};