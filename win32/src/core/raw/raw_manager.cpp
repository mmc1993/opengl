#include "raw_manager.h"
#include "../cfg/cfg_manager.h"
//  STB IMAGE 加载图片用
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "../third/stb_image.h"
#include "../third/assimp/postprocess.h"
#include "../third/assimp/Importer.hpp"
#include "../third/assimp/scene.h"

//  原始数据引用路径
const std::array<std::string, RawManager::kRawTypeEnum> RawManager::RAWDATA_REF = {
    {
        "res/raw/head.db",
        "res/raw/mesh.db",
        "res/raw/image.db",
        "res/raw/program.db",
        "res/raw/material.db",
        "res/raw/rawlisting.txt",
    }
};

//  文件后缀关联类型
const std::array<std::vector<std::string>, RawManager::kImportTypeEnum> RawManager::SUFFIX_MAP = {
    { 
        { ".obj", ".fbx" }, 
        { ".png", ".jpg" }, 
        { ".program" },
        { ".material" },
    }
};

void RawManager::Init()
{
    std::ifstream istream;
    //  头部信息
    istream.open(RAWDATA_REF[kRAW_HEAD], std::ios::binary);
    ASSERT_LOG(istream, "读取原始数据失败!: {0}", RAWDATA_REF[kRAW_HEAD]);

    RawHead::Head head;
    istream.read((char *)&head, sizeof(RawHead::Head));

    _rawHead.mMeshList.resize(head.mMeshLength * sizeof(RawHead::Info));
    istream.read((char *)_rawHead.mMeshList.data(), head.mMeshLength * sizeof(RawHead::Info));

    _rawHead.mImageList.resize(head.mImageLength * sizeof(RawHead::Info));
    istream.read((char *)_rawHead.mImageList.data(), head.mImageLength * sizeof(RawHead::Info));

    _rawHead.mProgramList.resize(head.mProgramLength * sizeof(RawHead::Info));
    istream.read((char *)_rawHead.mProgramList.data(), head.mProgramLength * sizeof(RawHead::Info));

    _rawHead.mMaterialList.resize(head.mMaterialLength * sizeof(RawHead::Info));
    istream.read((char *)_rawHead.mMaterialList.data(), head.mMaterialLength * sizeof(RawHead::Info));

    istream.close();

    ClearRawData();
}

void RawManager::BegImport()
{
    ClearRawData();
    _rawHead.mMeshList.clear();
    _rawHead.mImageList.clear();
    _rawHead.mProgramList.clear();
    _rawHead.mMaterialList.clear();
}

void RawManager::EndImport()
{
    std::ofstream os;
    //  写入网格
    os.open(RAWDATA_REF[kRAW_MESH], std::ios::binary);
    ASSERT_LOG(os, "导入Mesh失败. {0}", RAWDATA_REF[kRAW_MESH]);
    for (const auto & pair : _rawMeshMap)
    {
        auto byteOffset = os.tellp();
        os.write((const char *)&pair.second.mIndexLength, sizeof(uint));
        os.write((const char *)&pair.second.mVertexLength, sizeof(uint));
        os.write((const char *)pair.second.mIndexs, sizeof(uint) * pair.second.mIndexLength);
        os.write((const char *)pair.second.mVertexs, sizeof(float) * pair.second.mVertexLength);
        _rawHead.mMeshList.emplace_back(pair.first.c_str(), (uint)byteOffset, (uint)(os.tellp() - byteOffset));
    }
    os.close();

    //  写入图片
    os.open(RAWDATA_REF[kRAW_IMAGE], std::ios::binary);
    ASSERT_LOG(os, "导入Image失败. {0}", RAWDATA_REF[kRAW_IMAGE]);
    for (const auto & pair : _rawImageMap)
    {
        auto byteOffset = os.tellp();
        os.write((const char *)&pair.second.mW, sizeof(uint));
        os.write((const char *)&pair.second.mH, sizeof(uint));
        os.write((const char *)&pair.second.mFormat, sizeof(uint));
        os.write((const char *)&pair.second.mByteLength, sizeof(uint));
        os.write((const char *)pair.second.mData, pair.second.mByteLength);
        _rawHead.mImageList.emplace_back(pair.first.c_str(), (uint)byteOffset, (uint)(os.tellp() - byteOffset));
    }
    os.close();

    //  写入程序
    os.open(RAWDATA_REF[kRAW_PROGRAM], std::ios::binary);
    ASSERT_LOG(os, "导入Program失败. {0}", RAWDATA_REF[kRAW_PROGRAM]);
    for (const auto & pair : _rawProgramMap)
    {
        auto byteOffset = os.tellp();
        os.write((const char *)&pair.second.mPassLength, sizeof(uint));
        os.write((const char *)&pair.second.mVSByteLength, sizeof(uint));
        os.write((const char *)&pair.second.mGSByteLength, sizeof(uint));
        os.write((const char *)&pair.second.mFSByteLength, sizeof(uint));
        os.write((const char *)&pair.second.mData, pair.second.mVSByteLength 
                                                      + pair.second.mGSByteLength 
                                                      + pair.second.mFSByteLength
                                                      + sizeof(GLProgram::PassAttr) * pair.second.mPassLength);
        _rawHead.mProgramList.emplace_back(pair.first.c_str(), (uint)byteOffset, (uint)(os.tellp() - byteOffset));
    }
    os.close();

    //  写入材质
    os.open(RAWDATA_REF[kRAW_MATERIAL], std::ios::binary);
    ASSERT_LOG(os, "导入Material失败. {0}", RAWDATA_REF[kRAW_MATERIAL]);
    for (const auto & pair : _rawMaterialMap)
    {
        auto byteOffset = os.tellp();
        os.write((const char *)&pair.second, sizeof(RawMaterial));
        _rawHead.mMaterialList.emplace_back(pair.first.c_str(), (uint)byteOffset, (uint)(os.tellp() - byteOffset));
    }
    os.close();

    //  写入数据头文件
    RawHead::Head head;
    head.mMeshLength = _rawHead.mMeshList.size();
    head.mImageLength = _rawHead.mImageList.size();
    head.mProgramLength = _rawHead.mProgramList.size();
    head.mMaterialLength = _rawHead.mMaterialList.size();
    //  Data Write
    os.open(RAWDATA_REF[kRAW_HEAD], std::ios::binary);
    os.write((const char *)&head, sizeof(RawHead::Head));
    os.write((const char *)_rawHead.mMeshList.data(), head.mMeshLength * sizeof(RawHead::Info));
    os.write((const char *)_rawHead.mImageList.data(), head.mImageLength * sizeof(RawHead::Info));
    os.write((const char *)_rawHead.mProgramList.data(), head.mProgramLength * sizeof(RawHead::Info));
    os.write((const char *)_rawHead.mMaterialList.data(), head.mMaterialLength * sizeof(RawHead::Info));
    os.close();

    os.open(RAWDATA_REF[kRAW_LISTING]);
    for (const auto & pair : _rawListing)
    {
        os << SFormat("[{0}]={1}\n", pair.first, pair.second);
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
    ASSERT_LOG(type != kImportTypeEnum, "导入的资源格式非法!: {0}", url);
    Import(url, type);
}

bool RawManager::LoadRaw(const std::string & key)
{
    if (LoadRaw(key, RawTypeEnum::kRAW_MESH)) { return true; }
    if (LoadRaw(key, RawTypeEnum::kRAW_IMAGE)) { return true; }
    if (LoadRaw(key, RawTypeEnum::kRAW_PROGRAM)) { return true; }
    if (LoadRaw(key, RawTypeEnum::kRAW_MATERIAL)) { return true; }
    return false;
}

bool RawManager::LoadRaw(const std::string & key, RawTypeEnum type)
{
    //  数据容器
    std::vector<RawHead::Info> * list[] = {
        &_rawHead.mMeshList,
        &_rawHead.mImageList,
        &_rawHead.mProgramList,
        &_rawHead.mMaterialList,
    };
    
    auto it = std::find(list[type]->begin(), 
                        list[type]->end(), key);
    if (it == list[type]->end()) { return false; }
    
    //  加载函数
    void (RawManager::*func[])(std::ifstream &, const std::string &) = {
        &RawManager::LoadRawMesh,
        &RawManager::LoadRawImage,
        &RawManager::LoadRawProgram,
        &RawManager::LoadRawMaterial,
    };

    std::ifstream istream(RAWDATA_REF[type], std::ios::binary);
    ASSERT_LOG(istream, "找不到文件: {0}", RAWDATA_REF[type]);
    istream.seekg(it->mByteOffset, std::ios::beg);
    (this->*func[type])(istream, key);
    ASSERT_LOG((uint)istream.tellg() - it->mByteOffset == it->mByteLength, "文件读取长度不一致: {0}, {1}", type, key);
    istream.close();
    return true;
}

void RawManager::FreeRaw(const std::string & key)
{
    if (FreeRaw(key, kRAW_MESH)) return;
    if (FreeRaw(key, kRAW_IMAGE)) return;
    if (FreeRaw(key, kRAW_PROGRAM)) return;
    if (FreeRaw(key, kRAW_MATERIAL)) return;
}

bool RawManager::FreeRaw(const std::string & key, RawTypeEnum type)
{
    switch (type)
    {
    case RawManager::kRAW_MESH:
        {
            auto it = _rawMeshMap.find(key);
            if (it != _rawMeshMap.end())
            {
                delete[] it->second.mIndexs;
                delete[] it->second.mVertexs;
                _rawMeshMap.erase(it);
                return true;
            }
        }
        break;
    case RawManager::kRAW_IMAGE:
        {
            auto it = _rawImageMap.find(key);
            if (it != _rawImageMap.end())
            {
                stbi_image_free(it->second.mData);
                _rawImageMap.erase(it);
                return true;
            }
        }
        break;
    case RawManager::kRAW_PROGRAM:
        {
            auto it = _rawProgramMap.find(key);
            if (it != _rawProgramMap.end())
            {
                delete[] it->second.mData;
                _rawProgramMap.erase(it);
                return true;
            }
        }
        break;
    case RawManager::kRAW_MATERIAL:
        {
            auto it = _rawMaterialMap.find(key);
            if (it != _rawMaterialMap.end())
            {
                _rawMaterialMap.erase(it);
                return true;
            }
        }
        break;
    }
    return false;
}

void RawManager::Import(const std::string & url, ImportTypeEnum type)
{
    switch (type)
    {
    case RawManager::kIMPORT_MODEL: ImportModel(url); break;
    case RawManager::kIMPORT_IMAGE: ImportImage(url); break;
    case RawManager::kIMPORT_PROGRAM: ImportProgram(url); break;
    case RawManager::kIMPORT_MATERIAL: ImportMaterial(url); break;
    default: ASSERT_LOG(false, "导入的资源格式非法!: {0}, {1}", type, url); break;
    }
}

void RawManager::ImportModel(const std::string & url)
{
    std::function<void(aiNode * node, const aiScene * scene, const std::string & directory)> LoadNode;
    std::function<void(aiMesh * mesh, const aiScene * scene, const std::string & directory)> LoadImage;
    std::function<void(aiMesh * mesh, std::vector<float> & vertexs, std::vector<uint> & indexs)> LoadMesh;

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

    LoadMesh = [&, this](aiMesh * mesh, std::vector<float> & vertexs, std::vector<uint> & indexs)
    {
        for (auto i = 0; i != mesh->mNumVertices; ++i)
        {
            //	vertex
            vertexs.push_back(mesh->mVertices[i].x);
            vertexs.push_back(mesh->mVertices[i].y);
            vertexs.push_back(mesh->mVertices[i].z);
            //	normal
            vertexs.push_back(mesh->mNormals[i].x);
            vertexs.push_back(mesh->mNormals[i].y);
            vertexs.push_back(mesh->mNormals[i].z);
            //  color
            vertexs.push_back(1.0f);
            vertexs.push_back(1.0f);
            vertexs.push_back(1.0f);
            vertexs.push_back(1.0f);
            //	tan
            vertexs.push_back(mesh->mTangents[i].x);
            vertexs.push_back(mesh->mTangents[i].y);
            vertexs.push_back(mesh->mTangents[i].z);
            //	bitan
            vertexs.push_back(mesh->mBitangents[i].x);
            vertexs.push_back(mesh->mBitangents[i].y);
            vertexs.push_back(mesh->mBitangents[i].z);
            //	uv
            vertexs.push_back(mesh->mTextureCoords[0][i].x);
            vertexs.push_back(mesh->mTextureCoords[0][i].y);
        }
        if (!indexs.empty())
        {
            indexs.push_back((uint)~0);
        }
        for (auto i = 0; i != mesh->mNumFaces; ++i)
        {
            for (auto j = 0; j != mesh->mFaces[i].mNumIndices; ++j)
            {
                indexs.push_back(mesh->mFaces[i].mIndices[j]);
            }
        }
    };

    LoadNode = [&, this](aiNode * node, const aiScene * scene, const std::string & directory)
    {
        std::vector<uint> indexs;
        std::vector<float> vertexs;
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
        auto vertexByteLength = vertexs.size() * sizeof(float);
        rawMesh.mVertexLength = vertexs.size();
        rawMesh.mVertexs = new float[rawMesh.mVertexLength];
        memcpy(rawMesh.mVertexs, vertexs.data(), vertexByteLength);

        //  生成名字
        auto length = indexByteLength + vertexByteLength;
        auto buffer = new uchar[length];
        memcpy(buffer                  , rawMesh.mIndexs, indexByteLength);
        memcpy(buffer + indexByteLength, rawMesh.mVertexs, vertexByteLength);
        auto name = BuildName(buffer, length);
        delete[] buffer;

        _rawMeshMap.insert(std::make_pair(name, rawMesh));

        //  记录路径
        _rawListing.insert(std::make_pair(name, url));

        for (auto i = 0; i != node->mNumChildren; ++i)
        {
            LoadNode(node->mChildren[i], scene, directory);
        }
    };

    Assimp::Importer importer;
    auto scene = importer.ReadFile(url, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_FlipUVs);
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

    auto name = BuildName(rawImage.mData, rawImage.mByteLength);
    _rawImageMap.insert(std::make_pair(name, rawImage));
    
    //  记录路径
    _rawListing.insert(std::make_pair(name, url));
}

void RawManager::ImportProgram(const std::string & url)
{
    //  解析Include
    const auto ParseInclude = [](const std::string & word)
    {
        auto pos = word.find_first_of(' ');
        ASSERT_LOG(pos != std::string::npos, "Include Error: {0}", word);
        auto url = word.substr(pos + 1);

        std::ifstream istream(url);
        ASSERT_LOG(istream, "Include URL Error: {0}", url);

        std::string data;
        std::string line;
        while (std::getline(istream, line))
        {
            data.append(line);
            data.append("\n");
        }
        istream.close();
        return data;
    };

    //  解析Shader
    const auto ParseShader = [&](std::ifstream & is, const char * endflag, std::string & buffer)
    {
        std::string line;
        while (std::getline(is, line))
        {
            if (string_tool::IsEqualSkipSpace(line, endflag))
            {
                break;
            }
            if (string_tool::IsEqualSkipSpace(line, "#include"))
            {
                buffer.append(ParseInclude(line));
            }
            else
            {
                buffer.append(line);
                buffer.append("\n");
            }
        }
        ASSERT_LOG(string_tool::IsEqualSkipSpace(line, endflag), "EndFlag Error: {0}", endflag);
    };

    //  解析Pass
    const auto ParsePass = [&](
        std::ifstream & is, 
        const char * endFlag,
        std::string & vBuffer, 
        std::string & gBuffer, 
        std::string & fBuffer, 
        GLProgram::PassAttr * passAttr)
    {
        std::string line;
        while (std::getline(is, line))
        {
            if (string_tool::IsEqualSkipSpace(line, endFlag))
            {
                break;
            }
            if (string_tool::IsEqualSkipSpace(line, "#include"))
            {
                auto buffer = ParseInclude(line);
                vBuffer.append(buffer);
                gBuffer.append(buffer);
                fBuffer.append(buffer);
            }
            else if (string_tool::IsEqualSkipSpace(line, "CullFace") 
                || string_tool::IsEqualSkipSpace(line, "BlendMode")
                || string_tool::IsEqualSkipSpace(line, "DepthTest")
                || string_tool::IsEqualSkipSpace(line, "DepthWrite")
                || string_tool::IsEqualSkipSpace(line, "StencilTest")
                || string_tool::IsEqualSkipSpace(line, "RenderQueue")
                || string_tool::IsEqualSkipSpace(line, "RenderType")
                || string_tool::IsEqualSkipSpace(line, "DrawType"))
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
                else if (word == "DepthTest")
                {
                    passAttr->bDepthTest = true;
                }
                else if (word == "DepthWrite")
                {
                    passAttr->bDepthWrite = true;
                }
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
        }
        ASSERT_LOG(string_tool::IsEqualSkipSpace(line, endFlag), "EndFlag Error: {0}", endFlag);
    };

    std::ifstream is(url);
    ASSERT_LOG(is, "URL Error: {0}", url);

    //  解析GL Program数据
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
        if (string_tool::IsEqualSkipSpace(line, "Pass Common Beg"))
        {
            ParsePass(is, "Pass Common End", vCommonBuffer, gCommonBuffer, fCommonBuffer, nullptr);
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
    is.close();

    //  生成GL Program数据
    std::string vBuffer;
    std::string gBuffer;
    std::string fBuffer;
    auto attrs = new GLProgram::PassAttr[passs.size()];
    for (auto i = 0; i != passs.size(); ++i)
    {
        vBuffer.append(std::get<0>(passs.at(i)));
        gBuffer.append(std::get<1>(passs.at(i)));
        fBuffer.append(std::get<2>(passs.at(i)));
        memcpy(attrs + i, &std::get<3>(passs.at(i)), sizeof(GLProgram::PassAttr));
    }

    //  写入GL Program数据
    RawProgram rawProgram   = { 0 };
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

    auto byteLength = rawProgram.mPassLength * sizeof(GLProgram::PassAttr)
                    + rawProgram.mVSByteLength
                    + rawProgram.mGSByteLength
                    + rawProgram.mFSByteLength;
    rawProgram.mData = new uchar[byteLength];

    auto ptr = rawProgram.mData;
    memcpy(ptr, attrs, rawProgram.mPassLength * sizeof(GLProgram::PassAttr));
    ptr += rawProgram.mPassLength * sizeof(GLProgram::PassAttr);
    memcpy(ptr, vCommonBuffer.data(), vCommonBuffer.size());
    ptr += vCommonBuffer.size();
    memcpy(ptr, gCommonBuffer.data(), gCommonBuffer.size());
    ptr += gCommonBuffer.size();
    memcpy(ptr, fCommonBuffer.data(), fCommonBuffer.size());
    ptr += fCommonBuffer.size();
    delete[]attrs;

    auto name = BuildName(rawProgram.mData, byteLength);
    _rawProgramMap.insert(std::make_pair(name, rawProgram));

    //  记录路径
    _rawListing.insert(std::make_pair(name, url));
}

void RawManager::ImportMaterial(const std::string & url)
{
    std::ifstream is(url, std::ios::binary);
    ASSERT_LOG(is, "URL: {0}", url);
    ASSERT_LOG(sizeof(RawMaterial) == file_tool::GetFileLength(is), "URL: {0}", url);

    RawMaterial rawMaterial;
    is.read((char *)&rawMaterial, sizeof(RawMaterial));
    is.close();

    auto name = BuildName((uchar *)&rawMaterial, sizeof(RawMaterial));
    _rawMaterialMap.insert(std::make_pair(name, rawMaterial));

    //  记录路径
    _rawListing.insert(std::make_pair(name, url));
}

void RawManager::LoadRawMesh(std::ifstream & istream, const std::string & key)
{
    RawMesh rawMesh;
    istream.read((char *)&rawMesh, sizeof(RawMesh::mIndexLength) + sizeof(RawMesh::mVertexLength));
    rawMesh.mIndexs = new uint[rawMesh.mIndexLength];
    istream.read((char *)rawMesh.mIndexs, sizeof(uint) * rawMesh.mIndexLength);
    rawMesh.mVertexs = new float[rawMesh.mVertexLength];
    istream.read((char *)rawMesh.mVertexs, sizeof(float) * rawMesh.mVertexLength);

    _rawMeshMap.insert(std::make_pair(key, rawMesh));
}

void RawManager::LoadRawImage(std::ifstream & istream, const std::string & key)
{
    RawImage rawImage;
    istream.read((char *)&rawImage.mW, sizeof(uint));
    istream.read((char *)&rawImage.mH, sizeof(uint));
    istream.read((char *)&rawImage.mFormat, sizeof(uint));
    istream.read((char *)&rawImage.mByteLength, sizeof(uint));
    rawImage.mData = new uchar[rawImage.mByteLength];
    istream.read((char *)rawImage.mData, rawImage.mByteLength);

    _rawImageMap.insert(std::make_pair(key, rawImage));
}

void RawManager::LoadRawProgram(std::ifstream & istream, const std::string & key)
{
    RawProgram rawProgram;
    istream.read((char *)&rawProgram.mPassLength, sizeof(uint));
    istream.read((char *)&rawProgram.mVSByteLength, sizeof(uint));
    istream.read((char *)&rawProgram.mGSByteLength, sizeof(uint));
    istream.read((char *)&rawProgram.mFSByteLength, sizeof(uint));

    auto byteLength = rawProgram.mPassLength * sizeof(GLProgram::PassAttr)
                    + rawProgram.mVSByteLength
                    + rawProgram.mGSByteLength
                    + rawProgram.mFSByteLength;
    rawProgram.mData = new uchar[byteLength];
    
    istream.read((char *)rawProgram.mData, byteLength);

    _rawProgramMap.insert(std::make_pair(key, rawProgram));
}

void RawManager::LoadRawMaterial(std::ifstream & istream, const std::string & key)
{
    RawMaterial rawMaterial;
    istream.read((char *)&rawMaterial, sizeof(RawMaterial));
 
    _rawMaterialMap.insert(std::make_pair(key, rawMaterial));
}

void RawManager::ClearRawData()
{
    //  Delete Mesh Byte
    for (auto & rawMesh : _rawMeshMap)
    {
        delete[]rawMesh.second.mIndexs;
        delete[]rawMesh.second.mVertexs;
    }
    _rawMeshMap.clear();

    //  Delete Image Byte
    for (auto & rawImage : _rawImageMap)
    {
        stbi_image_free(rawImage.second.mData);
    }
    _rawImageMap.clear();

    //  Delete Program Byte
    for (auto & rawProgram : _rawProgramMap)
    {
        delete[]rawProgram.second.mData;
    }
    _rawProgramMap.clear();

    _rawMaterialMap.clear();
}

std::string RawManager::BuildName(const uchar * data, const uint len)
{
    const auto md5 = Code::MD5Encode(data, len);
    return Code::Base64Encode((const uchar *)md5.data(), md5.size());
}
