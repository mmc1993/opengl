#include "raw_manager.h"
#include "../cfg/cfg_manager.h"
//  STB IMAGE 加载图片用
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "../third/stb_image.h"
#include "../third/assimp/postprocess.h"
#include "../third/assimp/Importer.hpp"
#include "../third/assimp/scene.h"

const std::string RawManager::MANIFEST_SLOT_URL = "res/raw/manifest-slot.db";
const std::string RawManager::MANIFEST_INFO_URL = "res/raw/manifest-info.txt";

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
RawManager::RawMesh::RawMesh()
    : mIndexLength(0)
    , mVertexLength(0)
    , mIndexs(nullptr)
    , mVertexs(nullptr)
{ }

RawManager::RawMesh::~RawMesh()
{
    delete[] mIndexs;
    delete[] mVertexs;
}

void RawManager::RawMesh::Serialize(std::ofstream & os)
{
    os.write((const char *)&mIndexLength, sizeof(uint));
    os.write((const char *)&mVertexLength, sizeof(uint));
    os.write((const char *)mIndexs, sizeof(uint)            * mIndexLength);
    os.write((const char *)mVertexs, sizeof(GLMesh::Vertex) * mVertexLength);
}

void RawManager::RawMesh::Deserialize(std::ifstream & is)
{
    is.read((char *)&mIndexLength, sizeof(uint));
    is.read((char *)&mVertexLength, sizeof(uint));
    mIndexs = new uint[mIndexLength];
    is.read((char *)mIndexs, sizeof(uint) * mIndexLength);
    mVertexs = new GLMesh::Vertex[mVertexLength];
    is.read((char *)mVertexs, sizeof(GLMesh::Vertex) * mVertexLength);
}

//  Raw Image
RawManager::RawImage::RawImage()
    : mW(0), mH(0)
    , mFormat(0)
    , mByteLength(0)
    , mData(nullptr)
{ }

RawManager::RawImage::~RawImage()
{
    delete[] mData;
}

void RawManager::RawImage::Serialize(std::ofstream & os)
{
    os.write((const char *)&mW, sizeof(uint));
    os.write((const char *)&mH, sizeof(uint));
    os.write((const char *)&mFormat, sizeof(uint));
    os.write((const char *)&mByteLength, sizeof(uint));
    os.write((const char *)mData, mByteLength);
}

void RawManager::RawImage::Deserialize(std::ifstream & is)
{
    is.read((char *)&mW, sizeof(uint));
    is.read((char *)&mH, sizeof(uint));
    is.read((char *)&mFormat, sizeof(uint));
    is.read((char *)&mByteLength, sizeof(uint));
    mData = new uchar[mByteLength];
    is.read((char *)mData, mByteLength);
}

//  Raw Program
RawManager::RawProgram::RawProgram()
    : mData(nullptr)
    , mPassLength(0)
    , mVSByteLength(0)
    , mGSByteLength(0)
    , mFSByteLength(0)
{ }

RawManager::RawProgram::~RawProgram()
{
    delete[] mData;
}

void RawManager::RawProgram::Serialize(std::ofstream & os)
{
    os.write((const char *)&mPassLength, sizeof(uint));
    os.write((const char *)&mVSByteLength, sizeof(uint));
    os.write((const char *)&mGSByteLength, sizeof(uint));
    os.write((const char *)&mFSByteLength, sizeof(uint));
    os.write((const char *)mData, mVSByteLength + mGSByteLength + mFSByteLength
                                + mPassLength * sizeof(GLProgram::PassAttr));
}

void RawManager::RawProgram::Deserialize(std::ifstream & is)
{
    is.read((char *)&mPassLength, sizeof(uint));
    is.read((char *)&mVSByteLength, sizeof(uint));
    is.read((char *)&mGSByteLength, sizeof(uint));
    is.read((char *)&mFSByteLength, sizeof(uint));
    auto byteLength = mPassLength * sizeof(GLProgram::PassAttr)
                    + mVSByteLength + mGSByteLength + mFSByteLength;
    mData = new uchar[byteLength];
    is.read((char *)mData, byteLength);
}

//  Raw Material
RawManager::RawMaterial::RawMaterial()
    : mShininess(32)
{
    memset(mMesh, 0, RAW_NAME_LEN);
    memset(mProgram, 0, RAW_NAME_LEN);
    memset(mTextures, 0, MTLTEX2D_LEN * sizeof(Texture));
}

RawManager::RawMaterial::~RawMaterial()
{ }

void RawManager::RawMaterial::Serialize(std::ofstream & os)
{
    os.write((const char *)&mShininess, sizeof(uint));
    os.write((const char *)mMesh, RAW_NAME_LEN);
    os.write((const char *)mProgram, RAW_NAME_LEN);
    os.write((const char *)mTextures, MTLTEX2D_LEN * sizeof(Texture));
}

void RawManager::RawMaterial::Deserialize(std::ifstream & is)
{
    is.read((char *)&mShininess, sizeof(uint));
    is.read((char *)mMesh, RAW_NAME_LEN);
    is.read((char *)mProgram, RAW_NAME_LEN);
    is.read((char *)mTextures, MTLTEX2D_LEN * sizeof(Texture));
}

//  Raw Manager
void RawManager::Init()
{
    std::ifstream is(MANIFEST_SLOT_URL);
    ASSERT_LOG(is, "Open File Error. {0}", MANIFEST_SLOT_URL);

    auto length = file_tool::GetFileLength(is);
    ASSERT_LOG(length % sizeof(Manifest::Slot) == 0, "Manifest Slot Error.");
    
    _manifest.mSlots.resize(length / sizeof(Manifest::Slot));
    is.read((char *)_manifest.mSlots.data(), length);
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
        std::remove(MANIFEST_INFO_URL.c_str());
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
    std::ofstream os;
    
    os.open(MANIFEST_SLOT_URL, std::ios::binary);
    os.write((const char *)_manifest.mSlots.data(), _manifest.mSlots.size() * sizeof(Manifest::Slot));
    os.close();

    os.open(MANIFEST_INFO_URL);
    for (const auto & info : _manifest.mInfos)
    {
        os << SFormat("[{0}]={1}\n", info.mName, info.mURL);
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
    default: ASSERT_LOG(false, "Import Error. {0}, {1}", type, url); break;
    }
}

RawManager::Raw * RawManager::LoadRaw(const std::string & name)
{
    auto rawIt = _rawObjectMap.find(name);
    if (rawIt != _rawObjectMap.end())
    {
        return rawIt->second;
    }

    auto it = std::find(_manifest.mSlots.begin(), _manifest.mSlots.end(), name);
    ASSERT_LOG(it != _manifest.mSlots.end(), "Not Found Raw. {0}", name);

    auto raw = (Raw *)nullptr;
    switch (it->mType)
    {
    case kRAW_MESH: raw = new RawMesh(); break;
    case kRAW_IMAGE: raw = new RawImage(); break;
    case kRAW_PROGRAM: raw = new RawProgram(); break;
    case kRAW_MATERIAL: raw = new RawMaterial(); break;
    }
    ASSERT_LOG(raw != nullptr, "Type Error. {0}", it->mType);

    std::ifstream is(RAWDATA_URL[it->mType], std::ios::binary);
    ASSERT_LOG(is, "Open File Error. {0}", RAWDATA_URL[it->mType]);

    is.seekg(it->mByteOffset, std::ios::beg);
    raw->Deserialize(is);
    is.close();

    ASSERT_LOG((uint)is.tellg() - it->mByteOffset == it->mByteLength,
        "LengthErr. Name: {1}, Type: {0}, Length: {2}, ReadLength: {3}",
        name, it->mType, it->mByteLength, (uint)is.tellg() - it->mByteOffset);
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
    std::function<void(aiNode * node, const aiScene * scene, const std::string & directory)> LoadNode;
    std::function<void(aiMesh * mesh, const aiScene * scene, const std::string & directory)> LoadImage;
    std::function<void(aiMesh * mesh, std::vector<GLMesh::Vertex> & vertexs, std::vector<uint> & indexs)> LoadMesh;

    LoadImage = [&, this](aiMesh * mesh, const aiScene * scene, const std::string & directory)
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

    LoadMesh = [&, this](aiMesh * mesh, std::vector<GLMesh::Vertex> & vertexs, std::vector<uint> & indexs)
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

    LoadNode = [&, this](aiNode * node, const aiScene * scene, const std::string & directory)
    {
        std::vector<uint>           indexs;
        std::vector<GLMesh::Vertex> vertexs;
        for (auto i = 0; i != node->mNumMeshes; ++i)
        {
            LoadMesh(scene->mMeshes[node->mMeshes[i]], vertexs, indexs);

            LoadImage(scene->mMeshes[node->mMeshes[i]], scene, directory);
        }
        RawMesh rawMesh;
        //  索引数据
        auto indexByteLength = indexs.size() * sizeof(uint);
        rawMesh.mIndexLength = indexs.size();
        rawMesh.mIndexs = new uint[rawMesh.mIndexLength];
        memcpy(rawMesh.mIndexs, indexs.data(), indexByteLength);
        //  顶点数据
        auto vertexByteLength = vertexs.size() * sizeof(GLMesh::Vertex);
        rawMesh.mVertexLength = vertexs.size();
        rawMesh.mVertexs = new GLMesh::Vertex[rawMesh.mVertexLength];
        memcpy(rawMesh.mVertexs, vertexs.data(), vertexByteLength);
        //  生成名字
        auto length = indexByteLength + vertexByteLength;
        auto buffer = new uchar[length];
        memcpy(buffer                  , rawMesh.mIndexs, indexByteLength);
        memcpy(buffer + indexByteLength, rawMesh.mVertexs, vertexByteLength);
        auto name = BuildName(buffer, length);
        delete[] buffer;
        //  Write File
        std::ofstream os(RAWDATA_URL[kRAW_MESH], std::ios::binary | std::ios::app);
        ASSERT_LOG(os, "Import Model Failed. {0}", RAWDATA_URL[kRAW_MESH]);

        auto byteOffset = file_tool::GetFileLength(os);
        rawMesh.Serialize(os);
        auto byteLength = (uint)os.tellp() -byteOffset;
        os.close();

        _manifest.mSlots.emplace_back(name.c_str(), byteOffset, byteLength, kRAW_MESH);
        _manifest.mInfos.emplace_back(name.c_str(), url);

        for (auto i = 0; i != node->mNumChildren; ++i)
        {
            LoadNode(node->mChildren[i], scene, directory);
        }
    };

    Assimp::Importer importer;
    auto scene = importer.ReadFile(url, aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_FlipUVs);
    ASSERT_LOG(nullptr != scene, "Error URL: {0}", url);
    ASSERT_LOG(nullptr != scene->mRootNode, "Error URL: {0}", url);
    for (auto i = 0; i != scene->mRootNode->mNumChildren; ++i)
    {
        LoadNode(scene->mRootNode->mChildren[i], scene, url.substr(0, 1 + url.find_last_of('/')));
    }
}

void RawManager::ImportImage(const std::string & url)
{
    RawImage rawImage;
    rawImage.mData = stbi_load(url.c_str(),
        (int *)&rawImage.mW,
        (int *)&rawImage.mH,
        (int *)&rawImage.mFormat, 0);
    ASSERT_LOG(rawImage.mData != nullptr, "URL: {0}", url);

    rawImage.mByteLength = rawImage.mW * rawImage.mH * rawImage.mFormat;

    switch (rawImage.mFormat)
    {
    case 1: rawImage.mFormat = GL_RED; break;
    case 3: rawImage.mFormat = GL_RGB; break;
    case 4: rawImage.mFormat = GL_RGBA; break;
    }

    //  生成名字
    auto name = BuildName(rawImage.mData, rawImage.mByteLength);

    //  Write File
    std::ofstream os(RAWDATA_URL[kRAW_IMAGE], std::ios::binary | std::ios::app);
    ASSERT_LOG(os, "Import Image Failed. {0}", RAWDATA_URL[kRAW_IMAGE]);

    auto byteOffset = file_tool::GetFileLength(os);
    rawImage.Serialize(os);
    auto byteLength = (uint)os.tellp() - byteOffset;
    os.close();

    _manifest.mSlots.emplace_back(name.c_str(), byteOffset, byteLength, kRAW_IMAGE);
    _manifest.mInfos.emplace_back(name.c_str(), url);
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
    std::string vCommonBuffer;
    std::string gCommonBuffer;
    std::string fCommonBuffer;
    std::vector<std::tuple<
        std::string,
        std::string,
        std::string,
        GLProgram::PassAttr>> passs;
    while (std::getline(is, line))
    {
        if (string_tool::IsEqualSkipSpace(line, "Common Beg"))
        {
            ParsePass(is, "Common End", vCommonBuffer, gCommonBuffer, fCommonBuffer, nullptr);
        }
        else if (string_tool::IsEqualSkipSpace(line, "Pass Beg"))
        {
            decltype(passs)::value_type pass;
            ParsePass(is, "Pass End",
                std::get<0>(pass),  std::get<1>(pass),
                std::get<2>(pass), &std::get<3>(pass));
            passs.push_back(pass);
        }
    }

    //  生成GL Program数据
    std::string vBuffer, gBuffer, fBuffer;
    for (auto i = 0; i != passs.size(); ++i)
    {
        vBuffer.append(std::get<0>(passs.at(i)));
        gBuffer.append(std::get<1>(passs.at(i)));
        fBuffer.append(std::get<2>(passs.at(i)));
    }

    //  写入GL Program数据
    RawProgram rawProgram;
    rawProgram.mPassLength  = passs.size();
    if (!vBuffer.empty())
    {
        vCommonBuffer.append(vBuffer);
        rawProgram.mVSByteLength = vCommonBuffer.size();
    }
    if (!gBuffer.empty())
    {
        gCommonBuffer.append(gBuffer);
        rawProgram.mGSByteLength = gCommonBuffer.size();
    }
    if (!fBuffer.empty())
    {
        fCommonBuffer.append(fBuffer);
        rawProgram.mFSByteLength = fCommonBuffer.size();
    }

    auto dataLength = rawProgram.mPassLength * sizeof(GLProgram::PassAttr)
                    + rawProgram.mVSByteLength
                    + rawProgram.mGSByteLength
                    + rawProgram.mFSByteLength;
    rawProgram.mData = new uchar[dataLength];

    auto ptr = rawProgram.mData;
    for (auto i = 0; i != passs.size(); ++i)
    {
        memcpy(ptr, &std::get<3>(passs.at(i)), sizeof(GLProgram::PassAttr));
        ptr += sizeof(GLProgram::PassAttr);
    }
    if (rawProgram.mVSByteLength != 0)
    {
        memcpy(ptr, vCommonBuffer.data(), rawProgram.mVSByteLength);
        ptr += rawProgram.mVSByteLength;
    }
    if (rawProgram.mGSByteLength != 0)
    {
        memcpy(ptr, gCommonBuffer.data(), rawProgram.mGSByteLength);
        ptr += rawProgram.mGSByteLength;
    }
    if (rawProgram.mFSByteLength != 0)
    {
        memcpy(ptr, fCommonBuffer.data(), rawProgram.mFSByteLength);
        ptr += rawProgram.mFSByteLength;
    }
    ASSERT_LOG(ptr - rawProgram.mData == dataLength, "");

    //  生成名字
    auto name = BuildName(rawProgram.mData, dataLength);

    //  Write File
    std::ofstream os(RAWDATA_URL[kRAW_PROGRAM], std::ios::binary | std::ios::app);
    ASSERT_LOG(os, "Import Program Failed. {0}", RAWDATA_URL[kRAW_PROGRAM]);

    auto byteOffset = file_tool::GetFileLength(os);
    rawProgram.Serialize(os);
    auto byteLength = (uint)os.tellp() - byteOffset;
    os.close();

    _manifest.mSlots.emplace_back(name.c_str(), byteOffset, byteLength, kRAW_PROGRAM);
    _manifest.mInfos.emplace_back(name.c_str(), url);
}

void RawManager::ImportMaterial(const std::string & url)
{
    std::ifstream is(url);
    ASSERT_LOG(is, "URL: {0}", url);

    RawMaterial rawMaterial;
    is >> rawMaterial.mShininess;
    is >> rawMaterial.mMesh;
    is >> rawMaterial.mProgram;
    for (auto & texture : rawMaterial.mTextures)
    {
        is >> texture.mName;
        is >> texture.mTexture;
        if (is.eof()) break;
    }
    is.close();

    //  生成名字
    auto name = BuildName((uchar *)&rawMaterial, sizeof(RawMaterial));

    //  Write File
    std::ofstream os(RAWDATA_URL[kRAW_MATERIAL], std::ios::binary | std::ios::app);
    ASSERT_LOG(os, "Import Material Failed. {0}", RAWDATA_URL[kRAW_MATERIAL]);

    auto byteOffset = file_tool::GetFileLength(os);
    rawMaterial.Serialize(os);
    auto byteLength = (uint)os.tellp() - byteOffset;
    os.close();

    _manifest.mSlots.emplace_back(name.c_str(), byteOffset, byteLength, kRAW_MATERIAL);
    _manifest.mInfos.emplace_back(name.c_str(), url);
}

GLRes * RawManager::LoadResMesh(const std::string & name)
{
    auto raw = (RawMesh *)LoadRaw(name);
    ASSERT_LOG(raw != nullptr, "Not Found Raw. {0}", name);

    auto res = new GLMesh();
    res->Init(
        raw->mVertexs, raw->mVertexLength,
        raw->mIndexs,  raw->mIndexLength,
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
              GL_UNSIGNED_BYTE, raw->mW, raw->mH, raw->mData);
    _resObjectMap.insert(std::make_pair(name, res));
    return res;
}

GLRes * RawManager::LoadResProgram(const std::string & name)
{
    auto raw = (RawProgram *)LoadRaw(name);
    ASSERT_LOG(raw != nullptr, "Not Found Raw. {0}", name);

    auto res = new GLProgram();
    auto pAttr = (GLProgram::PassAttr *)raw->mData;
    auto vData = (const char *)(pAttr + raw->mPassLength);
    auto gData = vData + raw->mVSByteLength;
    auto fData = gData + raw->mGSByteLength;
    //  PassAttr
    for (auto i = 0; i != raw->mPassLength; ++i)
    {
        res->AddPassAttr(*pAttr++);
    }
    //  Shader
    res->Init(
        (const char *)vData, raw->mVSByteLength,
        (const char *)gData, raw->mGSByteLength,
        (const char *)fData, raw->mFSByteLength);
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
    for (auto i = 0; i != MTLTEX2D_LEN && raw->mTextures[i].mName[0] != '\0'; ++i)
    {
        res->SetTexture2D(LoadRes<GLTexture2D>(raw->mTextures[i].mTexture), raw->mTextures[i].mName, i);
    }
    _resObjectMap.insert(std::make_pair(name, res));
    return res;
}

std::string RawManager::BuildName(const uchar * data, const uint len)
{
    uchar name[RAW_NAME_LEN] = { 0 };
    auto md5 = Code::MD5Encode(data, len);
    return (const char *)number_tool::Conver16((const uchar *)md5.c_str(), md5.size(), name);
}
