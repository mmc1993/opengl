#pragma once

#include "../include.h"
#include "texture.h"
#include "material.h"
#include "../third/assimp/postprocess.h"
#include "../third/assimp/Importer.hpp"
#include "../third/assimp/scene.h"

class Mesh;
class Model;
class Shader;
class BitmapCube;

class File {
public:
	static Model * LoadModel(const std::string & url);
	static Mesh * LoadMesh(const std::string & url);
	static Shader * LoadShader(const std::string & url);
	static Bitmap * LoadBitmap(const std::string & url);
	static Texture LoadTexture(const std::string & url);
	static BitmapCube * LoadBitmapCube(const std::string & url);

private:
	static Model * LoadModel(aiNode * node, const aiScene * scene, const std::string & directory);
	static Mesh * LoadMesh(aiMesh * node, const aiScene * scene, const std::string & directory);
	static Material LoadMaterial(aiMesh * node, const aiScene * scene, const std::string & directory);

private:
	static std::string_view FindSubStrUntil(
		const std::string_view & str, 
		const std::string_view & end, 
		size_t pos = 0);
};