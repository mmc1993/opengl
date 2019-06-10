#include "raw_manager.h"
#include "../cfg/cfg_manager.h"
//  STB IMAGE 加载图片用
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "../third/stb_image.h"
#include "../third/assimp/scene.h"
#include "../third/assimp/Importer.hpp"
#include "../third/assimp/postprocess.h"

const std::string RawManager::MANIFEST_SLOT_URL = "res/raw/manifest-slot.db";

//  原始数据引用路径
const std::array<std::string, RawManager::kRawTypeEnum> RawManager::RAWDATA_URL = {
    {
        "res/raw/mesh.db",
        "res/raw/image.db",
        "res/raw/program.db",
        "res/raw/material.db",
    }
};

//  文件后缀关联类型
const std::array<std::vector<std::string>, RawManager::kImportTypeEnum> RawManager::SUFFIX_MAP = {
    { 
        { ".obj", ".fbx" }, 
        { ".png", ".jpg" }, 
        { ".program" },
        { ".mtl" },
    }
};

//  Raw Mesh
void RawManager::RawMesh::Serialize(std::ofstream & os)
{
    uint iSize = (uint)mIndexs.size();
    uint vSize = (uint)mVertexs.size();
    os.write((const char *)&iSize, sizeof(uint));
    os.write((const char *)&vSize, sizeof(uint));
    os.write((const char *)mIndexs.data(),  iSize * sizeof(decltype(mIndexs)::value_type));
    os.write((const char *)mVertexs.data(), vSize * sizeof(decltype(mIndexs)::value_type));
}

void RawManager::RawMesh::Deserialize(std::ifstream & is)
{
    uint iSize = 0;
    uint vSize = 0;
    is.read((char *)&iSize, sizeof(uint));
    is.read((char *)&vSize, sizeof(uint));

    mIndexs.resize(iSize);
    mVertexs.resize(vSize);
    is.read((char *)mIndexs.data(),  sizeof(decltype(mIndexs)::value_type) * iSize);
    is.read((char *)mVertexs.data(), sizeof(decltype(mVertexs)::value_type) * vSize);
}

//  Raw Image
void RawManager::RawImage::Serialize(std::ofstream & os)
{
    uint size = (uint)mData.size();
    os.write((const char *)&mW, sizeof(uint));
    os.write((const char *)&mH, sizeof(uint));
    os.write((const char *)&mFormat, sizeof(uint));
    os.write((const char *)&size, sizeof(uint));
    os.write((const char *)mData.c_str(), size);
}

void RawManager::RawImage::Deserialize(std::ifstream & is)
{
    uint size = 0;
    is.read((char *)&mW, sizeof(uint));
    is.read((char *)&mH, sizeof(uint));
    is.read((char *)&mFormat, sizeof(uint));

    is.read((char *)&size, sizeof(uint));
    mData.resize(size);
    is.read((char *)mData.data(), size);
}

//  Raw Program
void RawManager::RawProgram::Serialize(std::ofstream & os)
{
    uint8 aSize = (uint8)mAttrs.size();
    uint vSize  = (uint)mVSBuffer.size();
    uint gSize  = (uint)mGSBuffer.size();
    uint fSize  = (uint)mFSBuffer.size();

    os.write((const char *)&aSize, sizeof(uint8));
    os.write((const char *)&vSize, sizeof(uint));
    os.write((const char *)&gSize, sizeof(uint));
    os.write((const char *)&fSize, sizeof(uint));

    os.write((const char *)mAttrs.data(), sizeof(GLProgram::PassAttr) * aSize);
    if (!mVSBuffer.empty()) { os.write((const char *)mVSBuffer.c_str(), vSize); }
    if (!mGSBuffer.empty()) { os.write((const char *)mGSBuffer.c_str(), gSize); }
    if (!mFSBuffer.empty()) { os.write((const char *)mFSBuffer.c_str(), fSize); }
}

void RawManager::RawProgram::Deserialize(std::ifstream & is)
{
    uint8 aSize = 0;
    uint  vSize = 0, gSize = 0, fSize = 0;

    is.read((char *)&aSize, sizeof(uint8));
    is.read((char *)&vSize, sizeof(uint));
    is.read((char *)&gSize, sizeof(uint));
    is.read((char *)&fSize, sizeof(uint));

    mAttrs.resize(aSize); is.read((char *)mAttrs.data(), sizeof(GLProgram::PassAttr) * aSize);
    if (vSize != 0) { mVSBuffer.resize(vSize); is.read((char *)mVSBuffer.data(), vSize); }
    if (gSize != 0) { mGSBuffer.resize(gSize); is.read((char *)mGSBuffer.data(), gSize); }
    if (fSize != 0) { mFSBuffer.resize(fSize); is.read((char *)mFSBuffer.data(), fSize); }
}

//  Raw Material
void RawManager::RawMaterial::Serialize(std::ofstream & os)
{
    os.write((const char *)&mShininess, sizeof(uint));

    uint8 size = (uint8)mMesh.size();
    os.write((const char *)&size, sizeof(uint8));
    os.write((const char *)mMesh.c_str(), size);

    size = (uint8)mProgram.size();
    os.write((const char *)&size, sizeof(uint8));
    os.write((const char *)mProgram.c_str(), size);

    size = (uint8)mTexture2Ds.size();
    os.write((const char *)&size, sizeof(uint8));
    for (auto & texture : mTexture2Ds)
    {
        size = (uint8)texture.mName.size();
        os.write((const char *)&size, sizeof(uint8));
        os.write((const char *)texture.mName.c_str(), size);

        size = (uint8)texture.mDsec.size();
        os.write((const char *)&size, sizeof(uint8));
        os.write((const char *)texture.mDsec.c_str(), size);
    }
}

void RawManager::RawMaterial::Deserialize(std::ifstream & is)
{
    is.read((char *)&mShininess, sizeof(uint));

    uint8 size = 0;

    is.read((char *)&size, sizeof(uint8));
    mMesh.resize(size);
    is.read((char *)mMesh.data(), size);

    is.read((char *)&size, sizeof(uint8));
    mProgram.resize(size);
    is.read((char *)mProgram.data(), size);

    is.read((char *)&size, sizeof(uint8));
    mTexture2Ds.resize(size);
    for (auto & texture : mTexture2Ds)
    {
        is.read((char *)&size, sizeof(uint8));
        texture.mName.resize(size);
        is.read((char *)texture.mName.data(), size);

        is.read((char *)&size, sizeof(uint8));
        texture.mDsec.resize(size);
        is.read((char *)texture.mDsec.data(), size);
    }
}

//  Raw Manager
void RawManager::Init()
{
    std::ifstream is(MANIFEST_SLOT_URL);
    ASSERT_LOG(is, "Open File Error. {0}", MANIFEST_SLOT_URL);

    uint count = 0;
    is.read((char *)&count, sizeof(uint));
    _manifest.resize(count);

    for (auto & slot : _manifest)
    {
        is.read((char *)&slot.mByteOffset, sizeof(uint));
        is.read((char *)&slot.mByteLength, sizeof(uint));
        is.read((char *)&slot.mType, sizeof(RawTypeEnum));

        uint8 size = 0;
        is.read((char *) &size, sizeof(uint8));
        slot.mName.resize(size);
        is.read((char *)slot.mName.data(), size);
    }
    is.close();

    //  清空原始资源
    for (auto & pair : _rawObjectMap)
    {
        delete pair.second;
    }
    _rawObjectMap.clear();

    //  清空资源
    for (auto & res : _resObjectMap)
    {
        delete res.second;
    }
    _resObjectMap.clear();
}

void RawManager::BegImport(bool clear)
{
    if (clear)
    {
        for (auto & path : RAWDATA_URL)
        {
            std::remove(path.c_str());
        }
        std::remove(MANIFEST_SLOT_URL.c_str());
    }
    if (!file_tool::IsFileExists(MANIFEST_SLOT_URL))
    {
        file_tool::GenFile(MANIFEST_SLOT_URL, true);
    }
    Init();
}

void RawManager::EndImport()
{
    std::ofstream os(MANIFEST_SLOT_URL, std::ios::binary);
    
    uint count = (uint)_manifest.size();
    os.write((const char *)&count, sizeof(uint));

    for (auto & slot : _manifest)
    {
        os.write((const char *)&slot.mByteOffset, sizeof(uint));
        os.write((const char *)&slot.mByteLength, sizeof(uint));
        os.write((const char *)&slot.mType, sizeof(RawTypeEnum));

        uint8 size = (uint8)slot.mName.size();
        os.write((const char *)&size, sizeof(uint8));
        os.write((const char *)slot.mName.data(), size);
    }
    os.close();
}

void RawManager::Import(const std::string & url)
{
    ImportTypeEnum type = kImportTypeEnum;
    auto name = string_tool::QueryFileSuffix(url);
    for (auto i = 0u; i != SUFFIX_MAP.size(); ++i)
    {
        if (std::find(SUFFIX_MAP.at(i).begin(), 
                      SUFFIX_MAP.at(i).end(), name) != SUFFIX_MAP.at(i).end())
        {
            type = (ImportTypeEnum)i;
        }
    }
    ASSERT_LOG(type != kImportTypeEnum, "Import Error. {0}", url);
    switch (type)
    {
    case RawManager::kIMPORT_MODEL: ImportModel(url); break;
    case RawManager::kIMPORT_IMAGE: ImportImage(url); break;
    case RawManager::kIMPORT_PROGRAM: ImportProgram(url); break;
    case RawManager::kIMPORT_MATERIAL: ImportMaterial(url); break;
    }
}

RawManager::Raw * RawManager::LoadRaw(const std::string & name)
{
    auto rawIt = _rawObjectMap.find(name);
    if (rawIt != _rawObjectMap.end())
    {
        return rawIt->second;
    }
    auto it = std::find(_manifest.begin(), _manifest.end(), name);
    ASSERT_LOG(it != _manifest.end(), "Not Found Raw. {0}", name);

    auto raw = (Raw *)nullptr;
    switch (it->mType)
    {
    case kRAW_MESH: raw = new RawMesh(); break;
    case kRAW_IMAGE: raw = new RawImage(); break;
    case kRAW_PROGRAM: raw = new RawProgram(); break;
    case kRAW_MATERIAL: raw = new RawMaterial(); break;
    }
    ASSERT_LOG(raw != nullptr, "{0}", it->mType);

    std::ifstream is(RAWDATA_URL[it->mType], std::ios::binary);
    ASSERT_LOG(is, "{0}", RAWDATA_URL[it->mType]);

    is.seekg(it->mByteOffset, std::ios::beg);
    raw->Deserialize(is);
    auto byteLength = (uint)is.tellg() - it->mByteOffset;
    is.close();

    ASSERT_LOG(byteLength == it->mByteLength, "Name: {0}, Type: {1}, Length: {2}, Length: {3}", name, it->mType, it->mByteLength, byteLength);
    _rawObjectMap.insert(std::make_pair(name, raw));
    return raw;
}

void RawManager::FreeRaw(const std::string & name)
{
    auto it = _rawObjectMap.find(name);
    if (it != _rawObjectMap.end())
    {
        delete it->second; _rawObjectMap.erase(it);
    }
}

void RawManager::FreeRes(const std::string & name)
{
    auto it = _resObjectMap.find(name);
    if (it != _resObjectMap.end())
    {
        delete it->second; _resObjectMap.erase(it);
    }
}

void RawManager::ImportModel(const std::string & url)
{
    auto LoadImage = [this](aiMesh * mesh, const aiScene * scene, const std::string & directory)
    {
        aiString textureURL;
        auto aiMaterial = scene->mMaterials[mesh->mMaterialIndex];
        for (auto i = 0; i != aiMaterial->GetTextureCount(aiTextureType_DIFFUSE); ++i)
        {
            aiMaterial->GetTexture(aiTextureType_DIFFUSE, i, &textureURL);
            auto fullpath = directory + std::string(textureURL.C_Str());
            auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
            ImportImage(string_tool::Replace(urlpath, "\\", "/"));
        }

        if (0 != aiMaterial->GetTextureCount(aiTextureType_REFLECTION))
        {
            aiMaterial->GetTexture(aiTextureType_REFLECTION, 0, &textureURL);
            auto fullpath = directory + std::string(textureURL.C_Str());
            auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
            ImportImage(string_tool::Replace(urlpath, "\\", "/"));
        }

        if (0 != aiMaterial->GetTextureCount(aiTextureType_SPECULAR))
        {
            aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &textureURL);
            auto fullpath = directory + std::string(textureURL.C_Str());
            auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
            ImportImage(string_tool::Replace(urlpath, "\\", "/"));
        }

        if (0 != aiMaterial->GetTextureCount(aiTextureType_HEIGHT))
        {
            aiMaterial->GetTexture(aiTextureType_HEIGHT, 0, &textureURL);
            auto fullpath = directory + std::string(textureURL.C_Str());
            auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
            ImportImage(string_tool::Replace(urlpath, "\\", "/"));
        }
    };

    auto LoadMesh = [this](aiMesh * mesh, std::vector<GLMesh::Vertex> & vertexs, std::vector<uint> & indexs)
    {
        auto indexBase = vertexs.size();
        for (auto i = 0; i != mesh->mNumVertices; ++i)
        {
            GLMesh::Vertex vertex;
            //	position
            vertex.v.x = mesh->mVertices[i].x;
            vertex.v.y = mesh->mVertices[i].y;
            vertex.v.z = mesh->mVertices[i].z;
            //	normal
            vertex.n.x = mesh->mNormals[i].x;
            vertex.n.y = mesh->mNormals[i].y;
            vertex.n.z = mesh->mNormals[i].z;
            //  color
            vertex.c.r = 1.0f;
            vertex.c.g = 1.0f;
            vertex.c.b = 1.0f;
            vertex.c.a = 1.0f;
            //	tan
            vertex.tan.x = mesh->mTangents[i].x;
            vertex.tan.y = mesh->mTangents[i].y;
            vertex.tan.z = mesh->mTangents[i].z;
            //	bitan
            vertex.bitan.x = mesh->mBitangents[i].x;
            vertex.bitan.y = mesh->mBitangents[i].y;
            vertex.bitan.z = mesh->mBitangents[i].z;
            //	uv
            vertex.uv.x = mesh->mTextureCoords[0][i].x;
            vertex.uv.y = mesh->mTextureCoords[0][i].y;
            vertexs.push_back(vertex);
        }
        if (!indexs.empty())
        {
            indexs.push_back((uint)~0);
        }
        for (auto i = 0; i != mesh->mNumFaces; ++i)
        {
            std::transform(mesh->mFaces[i].mIndices,
                           mesh->mFaces[i].mIndices + mesh->mFaces[i].mNumIndices,
                           std::back_inserter(indexs), [&](const auto & v) { return v + indexBase; });
        }
    };

    auto LoadNode = [&](aiNode * node, const aiScene * scene, const std::string & directory, std::vector<GLMesh::Vertex> & vertexs, std::vector<uint> & indexs)
    {
        for (auto i = 0; i != node->mNumMeshes; ++i)
        {
            LoadMesh(scene->mMeshes[node->mMeshes[i]], vertexs, indexs);

            LoadImage(scene->mMeshes[node->mMeshes[i]], scene, directory);
        }
    };

    Assimp::Importer importer;
    auto scene = importer.ReadFile(url, aiProcess_JoinIdenticalVertices 
                                      | aiProcess_CalcTangentSpace 
                                      | aiProcess_Triangulate 
                                      | aiProcess_FlipUVs);
    ASSERT_LOG(nullptr != scene, "Error URL: {0}", url);
    ASSERT_LOG(nullptr != scene->mRootNode, "Error URL: {0}", url);

    std::vector<GLMesh::Vertex> vertexs;
    std::vector<uint>           indexs;
    std::queue<aiNode *>        nodes;
    nodes.push(scene->mRootNode);
    while (!nodes.empty())
    {
        auto node = nodes.front();
        nodes.pop();

        for (auto i = 0; i != node->mNumChildren; ++i)
        {
            nodes.push(scene->mRootNode->mChildren[i]);
        }
        LoadNode(node, scene, url.substr(0, 1 + url.find_last_of('/')), vertexs, indexs);
    }
   
    //  Write File
    std::ofstream os(RAWDATA_URL[kRAW_MESH], std::ios::binary | std::ios::app);
    ASSERT_LOG(os, "Import Model Failed. {0}", RAWDATA_URL[kRAW_MESH]);

    RawMesh rawMesh;
    rawMesh.mIndexs = std::move(indexs);
    rawMesh.mVertexs = std::move(vertexs);

    auto byteOffset = file_tool::GetFileLength(os);
    rawMesh.Serialize(os);
    auto byteLength = (uint)os.tellp() - byteOffset;
    os.close();

    _manifest.emplace_back(byteOffset, byteLength, kRAW_MESH, url);
}

void RawManager::ImportImage(const std::string & url)
{
    RawImage rawImage;
    auto data = stbi_load(url.c_str(),
        (int *)&rawImage.mW,
        (int *)&rawImage.mH,
        (int *)&rawImage.mFormat, 0);
    ASSERT_LOG(data != nullptr, "URL: {0}", url);

    auto length = rawImage.mW 
               * rawImage.mH 
               * rawImage.mFormat;
    rawImage.mData.resize(length);
    memcpy(rawImage.mData.data(), data, length);
    stbi_image_free(data);

    switch (rawImage.mFormat)
    {
    case 1: rawImage.mFormat = GL_RED; break;
    case 3: rawImage.mFormat = GL_RGB; break;
    case 4: rawImage.mFormat = GL_RGBA; break;
    }

    //  Write File
    std::ofstream os(RAWDATA_URL[kRAW_IMAGE], std::ios::binary | std::ios::app);
    ASSERT_LOG(os, "Import Image Failed. {0}", RAWDATA_URL[kRAW_IMAGE]);

    auto byteOffset = file_tool::GetFileLength(os);
    rawImage.Serialize(os);
    auto byteLength = (uint)os.tellp() - byteOffset;
    os.close();

    _manifest.emplace_back(byteOffset, byteLength, kRAW_IMAGE, url);
}

void RawManager::ImportProgram(const std::string & url)
{
    //  解析Shader
    const auto ParseShader = [&](
        std::stringstream & is, 
        const char * endflag, 
        std::string & buffer)
    {
        std::string line;
        while (std::getline(is, line))
        {
            if (string_tool::IsEqualSkipSpace(line, endflag))
            {
                break;
            }
            buffer.append(line);
            buffer.append("\n");
        }
        ASSERT_LOG(string_tool::IsEqualSkipSpace(line, endflag), "EndFlag Error: {0}", endflag);
    };

    //  解析Pass
    const auto ParsePass = [&](
        std::stringstream & is, 
        const char * endFlag,
        std::string & vBuffer, 
        std::string & gBuffer, 
        std::string & fBuffer, 
        GLProgram::PassAttr * passAttr)
    {
        std::string line;
        while (std::getline(is, line))
        {
            if (!string_tool::IsPrint(line)) { continue; }

            if (string_tool::IsEqualSkipSpace(line, endFlag))
            {
                break;
            }

            if (string_tool::IsEqualSkipSpace(line, "CullFace") 
                || string_tool::IsEqualSkipSpace(line, "BlendMode")
                || string_tool::IsEqualSkipSpace(line, "DepthTest")
                || string_tool::IsEqualSkipSpace(line, "DepthWrite")
                || string_tool::IsEqualSkipSpace(line, "StencilTest")
                || string_tool::IsEqualSkipSpace(line, "RenderQueue")
                || string_tool::IsEqualSkipSpace(line, "RenderType")
                || string_tool::IsEqualSkipSpace(line, "DrawType")
                || string_tool::IsEqualSkipSpace(line, "PassName"))
            {
                ASSERT_LOG(passAttr != nullptr, "解析Pass属性错误: {0}, {1}", endFlag, line);
                std::stringstream ss;
                std::string word;
                ss.str(line);
                ss >> word;
                
                if (word == "CullFace")
                {
                    ss >> word;
                    if (word == "Front")                    { passAttr->vCullFace = GL_FRONT; }
                    else if (word == "Back")                { passAttr->vCullFace = GL_BACK; }
                    else if (word == "FrontBack")           { passAttr->vCullFace = GL_FRONT_AND_BACK; }
                    else { ASSERT_LOG(false, "解析Pass属性错误: {0}, {1}", word, line); }
                }
                else if (word == "BlendMode")
                {
                    ss >> word;
                    if (word == "Zero")                     { passAttr->vBlendSrc = GL_ZERO; }
                    else if (word == "One")                 { passAttr->vBlendSrc = GL_ONE; }
                    else if (word == "SrcColor")            { passAttr->vBlendSrc = GL_SRC_COLOR; }
                    else if (word == "SrcAlpha")            { passAttr->vBlendSrc = GL_SRC_ALPHA; }
                    else if (word == "DstAlpha")            { passAttr->vBlendSrc = GL_DST_ALPHA; }
                    else if (word == "OneMinusSrcColor")    { passAttr->vBlendSrc = GL_ONE_MINUS_SRC_COLOR; }
                    else if (word == "OneMinusSrcAlpha")    { passAttr->vBlendSrc = GL_ONE_MINUS_SRC_ALPHA; }
                    else if (word == "OneMinusDstAlpha")    { passAttr->vBlendSrc = GL_ONE_MINUS_DST_ALPHA; }
                    else { ASSERT_LOG(false, "解析Pass属性错误: {0}, {1}", word, line); }

                    ss >> word;
                    if (word == "Zero")                     { passAttr->vBlendDst = GL_ZERO; }
                    else if (word == "One")                 { passAttr->vBlendDst = GL_ONE; }
                    else if (word == "SrcColor")            { passAttr->vBlendDst = GL_SRC_COLOR; }
                    else if (word == "SrcAlpha")            { passAttr->vBlendDst = GL_SRC_ALPHA; }
                    else if (word == "DstAlpha")            { passAttr->vBlendDst = GL_DST_ALPHA; }
                    else if (word == "OneMinusSrcColor")    { passAttr->vBlendDst = GL_ONE_MINUS_SRC_COLOR; }
                    else if (word == "OneMinusSrcAlpha")    { passAttr->vBlendDst = GL_ONE_MINUS_SRC_ALPHA; }
                    else if (word == "OneMinusDstAlpha")    { passAttr->vBlendDst = GL_ONE_MINUS_DST_ALPHA; }
                    else { ASSERT_LOG(false, "解析Pass属性错误: {0}, {1}", word, line); }
                }
                else if (word == "DepthTest")               { passAttr->bDepthTest = true; }
                else if (word == "DepthWrite")              { passAttr->bDepthWrite = true; }
                else if (word == "StencilTest")
                {
                    ss >> word;
                    if (word == "Keep")             { passAttr->vStencilOpFail = GL_KEEP; }
                    else if (word == "Zero")        { passAttr->vStencilOpFail = GL_ZERO; }
                    else if (word == "Incr")        { passAttr->vStencilOpFail = GL_INCR; }
                    else if (word == "Decr")        { passAttr->vStencilOpFail = GL_DECR; }
                    else if (word == "Invert")      { passAttr->vStencilOpFail = GL_INVERT; }
                    else if (word == "Replace")     { passAttr->vStencilOpFail = GL_REPLACE; }
                    else { ASSERT_LOG(false, "解析Pass属性错误: {0}, {1}", word, line); }

                    ss >> word;
                    if (word == "Keep")             { passAttr->vStencilOpZFail = GL_KEEP; }
                    else if (word == "Zero")        { passAttr->vStencilOpZFail = GL_ZERO; }
                    else if (word == "Incr")        { passAttr->vStencilOpZFail = GL_INCR; }
                    else if (word == "Decr")        { passAttr->vStencilOpZFail = GL_DECR; }
                    else if (word == "Invert")      { passAttr->vStencilOpZFail = GL_INVERT; }
                    else if (word == "Replace")     { passAttr->vStencilOpZFail = GL_REPLACE; }
                    else { ASSERT_LOG(false, "解析Pass属性错误: {0}, {1}", word, line); }

                    ss >> word;
                    if (word == "Keep")             { passAttr->vStencilOpZPass = GL_KEEP; }
                    else if (word == "Zero")        { passAttr->vStencilOpZPass = GL_ZERO; }
                    else if (word == "Incr")        { passAttr->vStencilOpZPass = GL_INCR; }
                    else if (word == "Decr")        { passAttr->vStencilOpZPass = GL_DECR; }
                    else if (word == "Invert")      { passAttr->vStencilOpZPass = GL_INVERT; }
                    else if (word == "Replace")     { passAttr->vStencilOpZPass = GL_REPLACE; }
                    else { ASSERT_LOG(false, "解析Pass属性错误: {0}, {1}", word, line); }

                    ss >> word;
                    if (word == "Never")            { passAttr->vStencilFunc = GL_NEVER; }
                    else if (word == "Less")        { passAttr->vStencilFunc = GL_LESS; }
                    else if (word == "Equal")       { passAttr->vStencilFunc = GL_EQUAL; }
                    else if (word == "Greater")     { passAttr->vStencilFunc = GL_GREATER; }
                    else if (word == "NotEqual")    { passAttr->vStencilFunc = GL_NOTEQUAL; }
                    else if (word == "Gequal")      { passAttr->vStencilFunc = GL_GEQUAL; }
                    else if (word == "Always")      { passAttr->vStencilFunc = GL_ALWAYS; }
                    else { ASSERT_LOG(false, "解析Pass属性错误: {0}, {1}", word, line); }

                    ss >> word;
                    passAttr->vStencilMask = std::stoi(word);

                    ss >> word;
                    passAttr->vStencilRef = std::stoi(word);
                }
                else if (word == "PassName")
                {
                    ss >> word;
                    memcpy(passAttr->mPassName, word.c_str(), word.size());
                }
                else if (word == "RenderQueue")
                {
                    ss >> word;
                    if (word == "Background")       { passAttr->vRenderQueue = 0; }
                    else if (word == "Geometric")   { passAttr->vRenderQueue = 1; }
                    else if (word == "Opacity")     { passAttr->vRenderQueue = 2; }
                    else if (word == "Top")         { passAttr->vRenderQueue = 3; }
                    else { ASSERT_LOG(false, "解析Pass属性错误: {0}, {1}", word, line); }
                }
                else if (word == "RenderType")
                {
                    ss >> word;
                    if (word == "Light")            { passAttr->vRenderType = 0; }
                    else if (word == "Shadow")      { passAttr->vRenderType = 1; }
                    else if (word == "Forward")     { passAttr->vRenderType = 2; }
                    else if (word == "Deferred")    { passAttr->vRenderType = 3; }
                    else { ASSERT_LOG(false, "解析Pass属性错误: {0}, {1}", word, line); }
                }
                else if (word == "DrawType")
                {
                    ss >> word;
                    if (word == "Instance")         { passAttr->vDrawType = 0; }
                    else if (word == "Vertex")      { passAttr->vDrawType = 1; }
                    else if (word == "Index")       { passAttr->vDrawType = 2; }
                    else { ASSERT_LOG(false, "解析Pass属性错误: {0}, {1}", word, line); }
                }
                else { ASSERT_LOG(false, "解析Pass属性错误: {0}, {1}", word, line); }
            }
            else if (string_tool::IsEqualSkipSpace(line, "VShader Beg"))
            {
                ParseShader(is, "VShader End", vBuffer);
            }
            else if (string_tool::IsEqualSkipSpace(line, "GShader Beg"))
            {
                ParseShader(is, "GShader End", gBuffer);
            }
            else if (string_tool::IsEqualSkipSpace(line, "FShader Beg"))
            {
                ParseShader(is, "FShader End", fBuffer);
            }
            else
            {
                vBuffer.append(line);
                vBuffer.append("\n");
                gBuffer.append(line);
                gBuffer.append("\n");
                fBuffer.append(line);
                fBuffer.append("\n");
            }
        }
        ASSERT_LOG(string_tool::IsEqualSkipSpace(line, endFlag), "EndFlag Error: {0}", endFlag);
    };

    //  解析Include
    const auto ParseInclude = [](const std::string & word)
    {
        auto pos = word.find_last_of(' ');
        ASSERT_LOG(pos != std::string::npos, "Include Error: {0}", word);
        auto url = word.substr(pos + 1);

        std::ifstream is(url);
        ASSERT_LOG(is, "Include URL Error: {0}", url);

        std::string data;
        std::string line;
        while (std::getline(is, line))
        {
            data.append(line);
            data.append("\n");
        }
        is.close();
        return data;
    };

    //  替换Include
    const auto OpenProgramFile = [&](const std::string & url)
    {
        std::string line,data;
        std::ifstream is(url);
        ASSERT_LOG(is, "URL Error: {0}", url);
        while (std::getline(is, line))
        {
            if (string_tool::IsEqualSkipSpace(line, "#include"))
            {
                data.append(ParseInclude(line));
                data.append("\n");
            }
            else
            {
                data.append(line);
                data.append("\n");
            }
        }
        is.close();

        std::stringstream ss;
        ss.str(data);
        return std::move(ss);
    };

    //  解析GL Program数据
    auto is = OpenProgramFile(url);
    std::string line;
    uint vLength = 0u;
    uint gLength = 0u;
    uint fLength = 0u;
    std::string vBuffer;
    std::string gBuffer;
    std::string fBuffer;
    std::vector<GLProgram::PassAttr> attrs;
    while (std::getline(is, line))
    {
        if (string_tool::IsEqualSkipSpace(line, "Common Beg"))
        {
            ParsePass(is, "Common End", vBuffer, gBuffer, fBuffer, nullptr);
        }
        else if (string_tool::IsEqualSkipSpace(line, "Pass Beg"))
        {
            GLProgram::PassAttr attr;
            auto vLen = vBuffer.size();
            auto gLen = gBuffer.size();
            auto fLen = fBuffer.size();
            ParsePass(is, "Pass End", vBuffer, gBuffer, fBuffer, &attr);
            if (vBuffer.size() != vLen) { vLength = vBuffer.size() - vLen; }
            if (gBuffer.size() != gLen) { gLength = gBuffer.size() - gLen; }
            if (fBuffer.size() != fLen) { fLength = fBuffer.size() - fLen; }
            attrs.push_back(attr);
        }
    }

    //  写入GL Program数据
    RawProgram rawProgram;
    rawProgram.mAttrs = std::move(attrs);
    if (vLength != 0) { rawProgram.mVSBuffer = std::move(vBuffer); }
    if (gLength != 0) { rawProgram.mGSBuffer = std::move(gBuffer); }
    if (fLength != 0) { rawProgram.mFSBuffer = std::move(fBuffer); }

    //  Write File
    std::ofstream os(RAWDATA_URL[kRAW_PROGRAM], std::ios::binary | std::ios::app);
    ASSERT_LOG(os, "Import Program Failed. {0}", RAWDATA_URL[kRAW_PROGRAM]);

    auto byteOffset = file_tool::GetFileLength(os);
    rawProgram.Serialize(os);
    auto byteLength = (uint)os.tellp() - byteOffset;
    os.close();

    _manifest.emplace_back(byteOffset, byteLength, kRAW_PROGRAM, url);
}

void RawManager::ImportMaterial(const std::string & url)
{
    std::ifstream is(url);
    ASSERT_LOG(is, "URL: {0}", url);

    RawMaterial rawMaterial;
    is >> rawMaterial.mShininess;
    is >> rawMaterial.mMesh;
    is >> rawMaterial.mProgram;
    while (!is.eof())
    {
        RawMaterial::Texture texture;
        is >> texture.mDsec;
        is >> texture.mName;
        rawMaterial.mTexture2Ds.push_back(texture);
    }
    is.close();

    //  Write File
    std::ofstream os(RAWDATA_URL[kRAW_MATERIAL], std::ios::binary | std::ios::app);
    ASSERT_LOG(os, "Import Material Failed. {0}", RAWDATA_URL[kRAW_MATERIAL]);

    auto byteOffset = file_tool::GetFileLength(os);
    rawMaterial.Serialize(os);
    auto byteLength = (uint)os.tellp() - byteOffset;
    os.close();

    _manifest.emplace_back(byteOffset, byteLength, kRAW_MATERIAL, url);
}

GLRes * RawManager::LoadResMesh(const std::string & name)
{
    auto raw = (RawMesh *)LoadRaw(name);
    ASSERT_LOG(raw != nullptr, "Not Found Raw. {0}", name);

    auto res = new GLMesh();
    res->Init(raw->mVertexs, raw->mIndexs,
              GLMesh::Vertex::kV_N_C_UV_TAN_BITAN);
    _resObjectMap.insert(std::make_pair(name, res));
    return res;
}

GLRes * RawManager::LoadResImage(const std::string & name)
{
    auto raw = (RawImage *)LoadRaw(name);
    ASSERT_LOG(raw != nullptr, "Not Found Raw. {0}", name);
    
    auto res = new GLTexture2D();
    res->Init(raw->mFormat, raw->mFormat, 
              GL_UNSIGNED_BYTE, raw->mW, raw->mH, 
              (const uchar *)raw->mData.data());
    res->SetParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    res->SetParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    res->SetParam(GL_TEXTURE_WRAP_S, GL_REPEAT);
    res->SetParam(GL_TEXTURE_WRAP_T, GL_REPEAT);
    _resObjectMap.insert(std::make_pair(name, res));
    return res;
}

GLRes * RawManager::LoadResProgram(const std::string & name)
{
    auto raw = (RawProgram *)LoadRaw(name);
    ASSERT_LOG(raw != nullptr, "Not Found Raw. {0}", name);

    auto res = new GLProgram();
    for (const auto & attr : raw->mAttrs)
    {
        res->AddPassAttr(attr);
    }
    res->Init(raw->mVSBuffer.c_str(), raw->mVSBuffer.size(),
              raw->mGSBuffer.c_str(), raw->mGSBuffer.size(),
              raw->mFSBuffer.c_str(), raw->mFSBuffer.size());
    _resObjectMap.insert(std::make_pair(name, res));
    return res;
}

GLRes * RawManager::LoadResMaterial(const std::string & name)
{
    auto raw = (RawMaterial *)LoadRaw(name);
    ASSERT_LOG(raw != nullptr, "Not Found Raw. {0}", name);
    
    auto res = new GLMaterial();
    res->SetShininess((float)raw->mShininess);
    res->SetMesh(LoadRes<GLMesh>(raw->mMesh));
    res->SetProgram(LoadRes<GLProgram>(raw->mProgram));
    for (auto i = 0; i != raw->mTexture2Ds.size(); ++i)
    {
        res->SetTexture2D(LoadRes<GLTexture2D>(
            raw->mTexture2Ds.at(i).mName), 
            raw->mTexture2Ds.at(i).mDsec, i);
    }
    _resObjectMap.insert(std::make_pair(name, res));
    return res;
}