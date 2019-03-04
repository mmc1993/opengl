#include "file.h"
#include "../mmc.h"
#include "model.h"
#include "shader.h"
#include "asset_cache.h"
#include "bitmap_cube.h"
#include "../tools/debug_tool.h"
#include "../tools/string_tool.h"
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "../third/stb_image.h"

#pragma comment(lib, "lib/assimp-vc140-mt.lib")

Model * File::LoadModel(const std::string & url)
{
	CHECK_RET(!mmc::mAssetCore.IsReg(url), mmc::mAssetCore.Get<Model>(url));

	Assimp::Importer importer;
	auto scene = importer.ReadFile(url,
								   aiProcess_CalcTangentSpace |
								   aiProcess_Triangulate |
								   aiProcess_FlipUVs);
	ASSERT_RET(scene != nullptr, nullptr);
	ASSERT_RET(scene->mRootNode != nullptr, nullptr);
	auto model = File::LoadModel(scene->mRootNode, scene, url.substr(0, 1 + url.find_last_of('/')));
	ASSERT_RET(model != nullptr, nullptr);
	mmc::mAssetCore.Reg(url, model);
	return model;
}

Shader * File::LoadShader(const std::string & url)
{
	CHECK_RET(!mmc::mAssetCore.IsReg(url), mmc::mAssetCore.Get<Shader>(url));

	auto vs = url + ".vs";
	auto fs = url + ".fs";
	auto gs = url + ".gs";
	std::ifstream vfile(vs);
	std::ifstream ffile(fs);
	std::ifstream gfile(gs);
	ASSERT_RET(vfile && ffile, nullptr);
	std::stringstream vss, fss, gss;
	vss << vfile.rdbuf();
	fss << ffile.rdbuf();
	gss << gfile.rdbuf();
	vfile.close();
	ffile.close();
	gfile.close();
	auto shader = new Shader(vss.str(), fss.str(), gss.str());
	mmc::mAssetCore.Reg(url, shader);
	assert(shader->GetGLID());
	return shader;
}

Bitmap * File::LoadBitmap(const std::string & url)
{
	CHECK_RET(!mmc::mAssetCore.IsReg(url), mmc::mAssetCore.Get<Bitmap>(url));
	
	auto w = 0, h = 0, c = 0, fmt = 0;
	auto buffer = stbi_load(url.c_str(), &w, &h, &c, 0);
	ASSERT_RET(buffer != nullptr, nullptr);
	switch (c)
	{
	case 1: fmt = GL_RED; break;
	case 3: fmt = GL_RGB; break;
	case 4: fmt = GL_RGBA; break;
	}
	auto bitmap = new Bitmap(w, h, fmt, fmt, GL_UNSIGNED_BYTE, url, buffer);
	bitmap->SetParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	bitmap->SetParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	bitmap->SetParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	bitmap->SetParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	mmc::mAssetCore.Reg(url, bitmap);
	stbi_image_free(buffer);
	return bitmap;
}

Texture File::LoadTexture(const std::string & url)
{
	return Texture(File::LoadBitmap(url));
}

BitmapCube * File::LoadBitmapCube(const std::string & url)
{
	CHECK_RET(!mmc::mAssetCore.IsReg(url), mmc::mAssetCore.Get<BitmapCube>(url));
	std::vector<std::string> urls{
		url + ".right.jpg",		url + ".left.jpg",
		url + ".top.jpg",		url + ".bottom.jpg",
		url + ".front.jpg",		url + ".back.jpg",
	};
	std::vector<const void*> buffers;
	auto w = 0, h = 0, fmt = 0;
	for (auto & url: urls)
	{
		auto buffer = stbi_load(url.c_str(), &w, &h, &fmt, 0);
		ASSERT_RET(buffer != nullptr, nullptr);
		buffers.push_back(buffer);
	}
	switch (fmt)
	{
	case 1: fmt = GL_RED; break;
	case 3: fmt = GL_RGB; break;
	case 4: fmt = GL_RGBA; break;
	}

	auto bitmapCube = new BitmapCube(w, h, fmt, fmt, GL_UNSIGNED_BYTE, urls, buffers);
	bitmapCube->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	bitmapCube->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	bitmapCube->SetParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	bitmapCube->SetParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	bitmapCube->SetParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	mmc::mAssetCore.Reg(url, bitmapCube);
	for (auto buffer : buffers)
	{
		stbi_image_free(const_cast<void *>(buffer));
	}
	return bitmapCube;
}

Model * File::LoadModel(aiNode * node, const aiScene * scene, const std::string & directory)
{
	auto model = new Model();
	for (auto i = 0; i != node->mNumMeshes; ++i)
	{
		model->mMeshs.emplace_back(File::LoadMesh(scene->mMeshes[node->mMeshes[i]], scene, directory));
		model->mMates.emplace_back(File::LoadMate(scene->mMeshes[node->mMeshes[i]], scene, directory));
	}
	for (auto i = 0; i != node->mNumChildren; ++i)
	{
		model->mChilds.emplace_back(File::LoadModel(node->mChildren[i], scene, directory));
	}
	return model;
}

Mesh * File::LoadMesh(aiMesh * mesh, const aiScene * scene, const std::string & directory)
{
	std::vector<std::uint32_t> indices;
	std::vector<Mesh::Vertex> vertexs;
	for (auto i = 0; i != mesh->mNumVertices; ++i)
	{
		Mesh::Vertex vertex;
		//	vertex
		vertex.v.x = mesh->mVertices[i].x;
		vertex.v.y = mesh->mVertices[i].y;
		vertex.v.z = mesh->mVertices[i].z;
		//	normal
		vertex.n.x = mesh->mNormals[i].x;
		vertex.n.y = mesh->mNormals[i].y;
		vertex.n.z = mesh->mNormals[i].z;
		//	tan
		vertex.tan.x = mesh->mTangents[i].x;
		vertex.tan.y = mesh->mTangents[i].y;
		vertex.tan.z = mesh->mTangents[i].z;
		//	bitan
		vertex.bitan.x = mesh->mBitangents[i].x;
		vertex.bitan.y = mesh->mBitangents[i].y;
		vertex.bitan.z = mesh->mBitangents[i].z;
		//	uv
		if (mesh->mTextureCoords[0])
		{
			vertex.uv.u = mesh->mTextureCoords[0][i].x;
			vertex.uv.v = mesh->mTextureCoords[0][i].y;
		}
		vertexs.push_back(vertex);
	}
	for (auto i = 0; i != mesh->mNumFaces; ++i)
	{
		for (auto j = 0; j != mesh->mFaces[i].mNumIndices; ++j)
		{
			indices.push_back(mesh->mFaces[i].mIndices[j]);
		}
	}
	return new Mesh(std::move(vertexs), std::move(indices));
}

Material File::LoadMate(aiMesh * mesh, const aiScene * scene, const std::string & directory)
{
	Material material;
	aiString textureURL;
	auto aiMaterial = scene->mMaterials[mesh->mMaterialIndex];
	for (auto i = 0; i != aiMaterial->GetTextureCount(aiTextureType_DIFFUSE); ++i)
	{
		aiMaterial->GetTexture(aiTextureType_DIFFUSE, i, &textureURL);
		auto fullpath = directory + std::string(textureURL.C_Str());
		auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
		material.mDiffuses.push_back(File::LoadTexture(urlpath));
	}

	if (0 != aiMaterial->GetTextureCount(aiTextureType_REFLECTION))
	{
		aiMaterial->GetTexture(aiTextureType_REFLECTION, 0, &textureURL);
		auto fullpath = directory + std::string(textureURL.C_Str());
		auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
		material.mReflect = File::LoadTexture(urlpath);
	}

	if (0 != aiMaterial->GetTextureCount(aiTextureType_SPECULAR))
	{
		aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &textureURL);
		auto fullpath = directory + std::string(textureURL.C_Str());
		auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
		material.mSpecular = File::LoadTexture(urlpath);
	}

	//if (0 != aiMaterial->GetTextureCount(aiTextureType_HEIGHT))
	//{
	//	aiMaterial->GetTexture(aiTextureType_HEIGHT, 0, &textureURL);
	//	auto fullpath = directory + std::string(textureURL.C_Str());
	//	auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
	//	material.mParallax = File::LoadTexture(urlpath);
	//}

	//if (0 != aiMaterial->GetTextureCount(aiTextureType_NORMALS))
	//{
	//	aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &textureURL);
	//	auto fullpath = directory + std::string(textureURL.C_Str());
	//	auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
	//	material.mNormal = File::LoadTexture(urlpath);
	//}

	if (0 != aiMaterial->GetTextureCount(aiTextureType_HEIGHT))
	{
		aiMaterial->GetTexture(aiTextureType_HEIGHT, 0, &textureURL);
		auto fullpath = directory + std::string(textureURL.C_Str());
		auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
		material.mNormal = File::LoadTexture(urlpath);
	}

	return std::move(material);
}
