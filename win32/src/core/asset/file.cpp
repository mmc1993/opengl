#include "file.h"
#include "../mmc.h"
#include "model.h"
#include "shader.h"
#include "asset_cache.h"
#include "bitmap_cube.h"
#include "../tools/debug_tool.h"
#include "../tools/string_tool.h"
#include "../render/render_type.h"
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

    std::ifstream ifile(url);
    ASSERT_RET(ifile, nullptr);

    auto shader = new Shader();
    std::string line;
    std::string word;
    std::string value;
    std::stringstream sstream;
    while (std::getline(ifile, line))
    {
        if (line == "Pass")
        {
            RenderPass pass;
            std::string fs, vs, gs;
            while (std::getline(ifile, line) && line != "End Pass")
            {
				word.clear();
                sstream.str(line);
				sstream.clear();
				sstream >> word;
                if (word == "CullFace")
                {
                    pass.bCullFace = true;
                    
                    sstream >> value;
                    if (value == "Front") { pass.vCullFace = GL_FRONT; }
                    else if (value == "Back") { pass.vCullFace = GL_BACK; }
                    else if (value == "FrontBack") { pass.vCullFace = GL_FRONT_AND_BACK; }
                }
                else if (word == "Blend")
                {
                    pass.bBlend = true;

                    sstream >> value;
                    if (value == "Zero") { pass.vBlendSrc = GL_ZERO; }
                    else if (value == "One") { pass.vBlendSrc = GL_ONE; }
                    else if (value == "SrcColor") { pass.vBlendSrc = GL_SRC_COLOR; }
                    else if (value == "SrcAlpha") { pass.vBlendSrc = GL_SRC_ALPHA; }
                    else if (value == "DstAlpha") { pass.vBlendSrc = GL_DST_ALPHA; }
                    else if (value == "OneMinusSrcColor") { pass.vBlendSrc = GL_ONE_MINUS_SRC_COLOR; }
                    else if (value == "OneMinusSrcAlpha") { pass.vBlendSrc = GL_ONE_MINUS_SRC_ALPHA; }
                    else if (value == "OneMinusDstAlpha") { pass.vBlendSrc = GL_ONE_MINUS_DST_ALPHA; }

                    sstream >> value;
                    if (value == "Zero") { pass.vBlendDst = GL_ZERO; }
                    else if (value == "One") { pass.vBlendDst = GL_ONE; }
                    else if (value == "SrcColor") { pass.vBlendDst = GL_SRC_COLOR; }
                    else if (value == "SrcAlpha") { pass.vBlendDst = GL_SRC_ALPHA; }
                    else if (value == "DstAlpha") { pass.vBlendDst = GL_DST_ALPHA; }
                    else if (value == "OneMinusSrcColor") { pass.vBlendDst = GL_ONE_MINUS_SRC_COLOR; }
                    else if (value == "OneMinusSrcAlpha") { pass.vBlendDst = GL_ONE_MINUS_SRC_ALPHA; }
                    else if (value == "OneMinusDstAlpha") { pass.vBlendDst = GL_ONE_MINUS_DST_ALPHA; }
                }
                else if (word == "DepthTest")
                {
                    pass.bDepthTest = true;
                }
                else if (word == "DepthWrite")
                {
                    pass.bDepthWrite = true;
                }
                else if (word == "StencilTest")
                {
                    pass.bStencilTest = true;

                    sstream >> value;
                    if (value == "Keep") { pass.vStencilOpFail = GL_KEEP; }
                    else if (value == "Zero") { pass.vStencilOpFail = GL_ZERO; }
                    else if (value == "Incr") { pass.vStencilOpFail = GL_INCR; }
                    else if (value == "Decr") { pass.vStencilOpFail = GL_DECR; }
                    else if (value == "Invert") { pass.vStencilOpFail = GL_INVERT; }
                    else if (value == "Replace") { pass.vStencilOpFail = GL_REPLACE; }

                    sstream >> value;
                    if (value == "Keep") { pass.vStencilOpZFail = GL_KEEP; }
                    else if (value == "Zero") { pass.vStencilOpZFail = GL_ZERO; }
                    else if (value == "Incr") { pass.vStencilOpZFail = GL_INCR; }
                    else if (value == "Decr") { pass.vStencilOpZFail = GL_DECR; }
                    else if (value == "Invert") { pass.vStencilOpZFail = GL_INVERT; }
                    else if (value == "Replace") { pass.vStencilOpZFail = GL_REPLACE; }

                    sstream >> value;
                    if (value == "Keep") { pass.vStencilOpZPass = GL_KEEP; }
                    else if (value == "Zero") { pass.vStencilOpZPass = GL_ZERO; }
                    else if (value == "Incr") { pass.vStencilOpZPass = GL_INCR; }
                    else if (value == "Decr") { pass.vStencilOpZPass = GL_DECR; }
                    else if (value == "Invert") { pass.vStencilOpZPass = GL_INVERT; }
                    else if (value == "Replace") { pass.vStencilOpZPass = GL_REPLACE; }

					sstream >> value;
					if (value == "Never") { pass.vStencilFunc = GL_NEVER; }
					else if (value == "Less") { pass.vStencilFunc = GL_LESS; }
					else if (value == "Equal") { pass.vStencilFunc = GL_EQUAL; }
					else if (value == "Greater") { pass.vStencilFunc = GL_GREATER; }
					else if (value == "NotEqual") { pass.vStencilFunc = GL_NOTEQUAL; }
					else if (value == "Gequal") { pass.vStencilFunc = GL_GEQUAL; }
					else if (value == "Always") { pass.vStencilFunc = GL_ALWAYS; }

                    sstream >> value;
                    pass.vStencilMask = std::stoi(value);

                    sstream >> value;
                    pass.vStencilRef = std::stoi(value);
                }
                else if (word == "RenderQueue")
                {
                    sstream >> value;
                    if (value == "Background") { pass.mRenderQueue = RenderQueueEnum::kBACKGROUND; }
                    else if (value == "Geometric") { pass.mRenderQueue = RenderQueueEnum::kGEOMETRIC; }
                    else if (value == "Opacity") { pass.mRenderQueue = RenderQueueEnum::kOPACITY; }
                    else if (value == "Top") { pass.mRenderQueue = RenderQueueEnum::kTOP; }
                }
                else if (word == "RenderType")
                {
					sstream >> value;
                    if (value == "Shadow") { pass.mRenderType = RenderTypeEnum::kSHADOW; }
                    else if (value == "Forward") { pass.mRenderType = RenderTypeEnum::kFORWARD; }
                    else if (value == "Deferred") { pass.mRenderType = RenderTypeEnum::kDEFERRED; }
                }
				else if (word == "DrawType")
				{
					sstream >> value;
					if (value == "Instance") { pass.mDrawType = DrawTypeEnum::kINSTANCE; }
					else if (value == "Vertex") { pass.mDrawType = DrawTypeEnum::kVERTEX; }
					else if (value == "Index") { pass.mDrawType = DrawTypeEnum::kINDEX; }
				}
                else if (word == "Vertex")
                {
                    std::vector<std::string> codes;
                    while (std::getline(ifile, line) && line != "End Vertex")
                    {
                        codes.emplace_back(std::move(line));
                    }
                    vs = string_tool::Join(codes, "\n");
                }
                else if (word == "Geometric")
                {
                    std::vector<std::string> codes;
                    while (std::getline(ifile, line) && line != "End Geometric")
                    {
                        codes.emplace_back(std::move(line));
                    }
                    gs = string_tool::Join(codes, "\n");
                }
                else if (word == "Fragment")
                {
                    std::vector<std::string> codes;
                    while (std::getline(ifile, line) && line != "End Fragment")
                    {
                        codes.emplace_back(std::move(line));
                    }
                    fs = string_tool::Join(codes, "\n");
                }
                else
                {
                    ASSERT_LOG(word.empty(), "Error Key Word: {0}", word);
                }
            }
            ASSERT_LOG(line == "End Pass", "Not Found End Pass");

            auto ret = shader->AddPass(pass, vs, fs, gs);
            ASSERT_LOG(ret, "Shader Pass Error");
        }
    }
    ASSERT_LOG(!shader->IsEmpty(), "Empty Shader Pass");
    mmc::mAssetCore.Reg(url, shader);
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
		material.mDiffuses.push_back(File::LoadBitmap(urlpath));
	}

	if (0 != aiMaterial->GetTextureCount(aiTextureType_REFLECTION))
	{
		aiMaterial->GetTexture(aiTextureType_REFLECTION, 0, &textureURL);
		auto fullpath = directory + std::string(textureURL.C_Str());
		auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
		material.mReflect = File::LoadBitmap(urlpath);
	}

	if (0 != aiMaterial->GetTextureCount(aiTextureType_SPECULAR))
	{
		aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &textureURL);
		auto fullpath = directory + std::string(textureURL.C_Str());
		auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
		material.mSpecular = File::LoadBitmap(urlpath);
	}

	if (0 != aiMaterial->GetTextureCount(aiTextureType_HEIGHT))
	{
		aiMaterial->GetTexture(aiTextureType_HEIGHT, 0, &textureURL);
		auto fullpath = directory + std::string(textureURL.C_Str());
		auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
		material.mNormal = File::LoadBitmap(urlpath);
	}

	return std::move(material);
}

RenderMesh File::LoadMesh(aiMesh * mesh, const aiScene * scene, const std::string & directory)
{
	std::vector<std::uint32_t>		indices;
	std::vector<RenderMesh::Vertex> vertexs;
	for (auto i = 0; i != mesh->mNumVertices; ++i)
	{
		RenderMesh::Vertex vertex;
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
	return RenderMesh::Create(vertexs, indices);
}