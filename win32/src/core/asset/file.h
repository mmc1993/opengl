#pragma once

#include "../include.h"
#include "texture.h"
#include "material.h"
#include "../third/assimp/postprocess.h"
#include "../third/assimp/Importer.hpp"
#include "../third/assimp/scene.h"
#include "../render/render_type.h"

class Model;
class Shader;
class BitmapCube;

class File {
public:
	static Model * LoadModel(const std::string & url);
	static Shader * LoadShader(const std::string & url);
	static Bitmap * LoadBitmap(const std::string & url);
	static Texture LoadTexture(const std::string & url);
	static BitmapCube * LoadBitmapCube(const std::string & url);

private:
	static Model * LoadModel(aiNode * node, const aiScene * scene, const std::string & directory);
	static Material LoadMate(aiMesh * mesh, const aiScene * scene, const std::string & directory);
	static RenderMesh LoadMesh(aiMesh * mesh, const aiScene * scene, const std::string & directory);
};