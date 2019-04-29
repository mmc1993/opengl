#pragma once

#include "../include.h"
#include "pass.h"
#include "mesh.h"
#include "model.h"
#include "shader.h"
#include "material.h"
#include "bitmap.h"
#include "bitmap_cube.h"
#include "../third/assimp/postprocess.h"
#include "../third/assimp/Importer.hpp"
#include "../third/assimp/scene.h"

class Model;
class Shader;
class BitmapCube;

class File {
public:
	static Model * LoadModel(const std::string & url);
	static Shader * LoadShader(const std::string & url);
	static Bitmap * LoadBitmap(const std::string & url);
	static BitmapCube * LoadBitmapCube(const std::string & url);

private:
	static Model * LoadModel(aiNode * node, const aiScene * scene, const std::string & directory);
	static Material LoadMate(aiMesh * mesh, const aiScene * scene, const std::string & directory);
	static Mesh LoadMesh(aiMesh * mesh, const aiScene * scene, const std::string & directory);
};