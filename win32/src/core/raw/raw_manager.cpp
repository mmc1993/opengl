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
        "raw/head.db",
        "raw/mesh.db",
        "raw/image.db",
        "raw/program.db",
        "raw/material.db",
    }
};

//  文件后缀关联类型
const std::array<std::vector<std::string>, RawManager::kImportTypeEnum> RawManager::SUFFIX_MAP = {
    { 
        { ".obg", ".fbx" }, 
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
    std::ofstream ostream;
    //  写入网格
    ostream.open(RAWDATA_REF[kRAW_MESH], std::ios::binary);
    ASSERT_LOG(ostream, "导入Mesh失败. {0}", RAWDATA_REF[kRAW_MESH]);
    for (const auto & pair : _rawMeshMap)
    {
        auto byteOffset = ostream.tellp();
        ostream.write((const char *)&pair.second.mIndexLength, sizeof(RawMesh::mIndexLength));
        ostream.write((const char *)&pair.second.mVertexLength, sizeof(RawMesh::mVertexLength));
        ostream.write((const char *)pair.second.mIndexs, sizeof(uint) * pair.second.mIndexLength);
        ostream.write((const char *)pair.second.mVertexs, sizeof(float) * pair.second.mVertexLength);
        auto byteLength = ostream.tellp() - byteOffset;
        ASSERT_LOG(byteLength == sizeof(pair.second.mIndexLength) 
                               + sizeof(pair.second.mVertexLength) 
                               + sizeof(uint) * pair.second.mIndexLength
                               + sizeof(float) * pair.second.mVertexLength, "数据长度不一致");
        _rawHead.mMeshList.emplace_back(pair.first.c_str(), (uint)byteOffset, (uint)byteLength);
    }
    ostream.close();

    //  写入图片
    ostream.open(RAWDATA_REF[kRAW_IMAGE], std::ios::binary);
    ASSERT_LOG(ostream, "导入Image失败. {0}", RAWDATA_REF[kRAW_IMAGE]);
    for (const auto & pair : _rawImageMap)
    {
        auto byteOffset = ostream.tellp();
        ostream.write((const char *)&pair.second.mW, sizeof(RawImage::mW));
        ostream.write((const char *)&pair.second.mH, sizeof(RawImage::mH));
        ostream.write((const char *)&pair.second.mFormat, sizeof(RawImage::mFormat));
        ostream.write((const char *)&pair.second.mLength, sizeof(RawImage::mLength));
        ostream.write((const char *)pair.second.mData, pair.second.mLength * sizeof(uchar));
        auto byteLength = ostream.tellp() - byteOffset;
        ASSERT_LOG(byteLength == sizeof(pair.second.mW)
                               + sizeof(pair.second.mH)
                               + sizeof(pair.second.mFormat)
                               + sizeof(pair.second.mLength)
                               + sizeof(uchar) * pair.second.mLength, "数据长度不一致");
        _rawHead.mImageList.emplace_back(pair.first.c_str(), (uint)byteOffset, (uint)byteLength);
    }
    ostream.close();

    //  写入程序
    ostream.open(RAWDATA_REF[kRAW_PROGRAM], std::ios::binary);
    ASSERT_LOG(ostream, "导入Program失败. {0}", RAWDATA_REF[kRAW_PROGRAM]);
    for (const auto & pair : _rawProgramMap)
    {
        auto byteOffset = ostream.tellp();
        ostream.write((const char *)&pair.second.mLength, sizeof(uint));
        ostream.write((const char *)&pair.second.mData, sizeof(uchar) * pair.second.mLength);
        auto byteLength = ostream.tellp() - byteOffset;
        ASSERT_LOG(byteLength == sizeof(pair.second.mLength)
                               + sizeof(uchar) * pair.second.mLength, "数据长度不一致");
        _rawHead.mProgramList.emplace_back(pair.first.c_str(), (uint)byteOffset, (uint)byteLength);
    }
    ostream.close();

    //  写入材质
    ostream.open(RAWDATA_REF[kRAW_MATERIAL], std::ios::binary);
    ASSERT_LOG(ostream, "导入Material失败. {0}", RAWDATA_REF[kRAW_MATERIAL]);
    for (const auto & pair : _rawMaterialMap)
    {
        auto byteOffset = ostream.tellp();
        ostream.write((const char *)&pair.second, sizeof(RawMaterial));
        auto byteLength = ostream.tellp() - byteOffset;
        ASSERT_LOG(byteLength == sizeof(RawMaterial), "数据长度不一致");
        _rawHead.mMaterialList.emplace_back(pair.first.c_str(), (uint)byteOffset, (uint)byteLength);
    }
    ostream.close();

    //  写入数据头文件
    RawHead::Head head;
    //  Head Mesh
    uint offset = 0;
    head.mMeshOffset = offset;
    head.mMeshLength = _rawHead.mMeshList.size();
    //  Head Image
    offset += _rawHead.mMeshList.size();
    head.mImageOffset = offset;
    head.mImageLength = _rawHead.mImageList.size();
    //  Head Program
    offset += _rawHead.mImageList.size();
    head.mProgramOffset = offset;
    head.mProgramLength = _rawHead.mProgramList.size();
    //  Head Material
    offset += _rawHead.mProgramList.size();
    head.mMaterialOffset = offset;
    head.mMaterialLength = _rawHead.mMaterialList.size();
    //  Data Write
    ostream.open(RAWDATA_REF[kRAW_HEAD], std::ios::binary);
    ostream.write((const char *)&head, sizeof(RawHead::Head));
    ostream.write((const char *)_rawHead.mMeshList.data(), head.mMeshLength * sizeof(RawHead::Info));
    ostream.write((const char *)_rawHead.mImageList.data(), head.mImageLength * sizeof(RawHead::Info));
    ostream.write((const char *)_rawHead.mProgramList.data(), head.mProgramLength * sizeof(RawHead::Info));
    ostream.write((const char *)_rawHead.mMaterialList.data(), head.mMaterialLength * sizeof(RawHead::Info));
    ostream.close();
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
                delete[] it->second.mData;
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
    }
    ASSERT_LOG(false, "导入的资源格式非法!: {0}, {1}", type, url);
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
            ImportImage(urlpath);
        }

        if (0 != aiMaterial->GetTextureCount(aiTextureType_REFLECTION))
        {
            aiMaterial->GetTexture(aiTextureType_REFLECTION, 0, &textureURL);
            auto fullpath = directory + std::string(textureURL.C_Str());
            auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
            ImportImage(urlpath);
        }

        if (0 != aiMaterial->GetTextureCount(aiTextureType_SPECULAR))
        {
            aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &textureURL);
            auto fullpath = directory + std::string(textureURL.C_Str());
            auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
            ImportImage(urlpath);
        }

        if (0 != aiMaterial->GetTextureCount(aiTextureType_HEIGHT))
        {
            aiMaterial->GetTexture(aiTextureType_HEIGHT, 0, &textureURL);
            auto fullpath = directory + std::string(textureURL.C_Str());
            auto urlpath = string_tool::Replace(fullpath, "\\\\", "/");
            ImportImage(urlpath);
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

        //  计算MD5
        auto length = indexByteLength + vertexByteLength;
        auto buffer = new char[length];
        memcpy(buffer, rawMesh.mIndexs, indexByteLength);
        memcpy(buffer, rawMesh.mVertexs, vertexByteLength);
        auto md5 = MD5(buffer, length);
        delete[] buffer;
        _rawMeshMap.insert(std::make_pair(md5.str, rawMesh));

        for (auto i = 0; i != node->mNumChildren; ++i)
        {
            LoadNode(node->mChildren[i], scene, directory);
        }
    };

    Assimp::Importer importer;
    auto scene = importer.ReadFile(url, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_FlipUVs);
    ASSERT_LOG(nullptr != scene, "Error URL: {0}", url);
    ASSERT_LOG(nullptr != scene->mRootNode, "Error URL: {0}", url);
    LoadNode(scene->mRootNode, scene, url.substr(0, 1 + url.find_last_of('/')));
}

void RawManager::ImportImage(const std::string & url)
{
    RawImage rawImage;
    rawImage.mData = stbi_load(url.c_str(),
        (int *)&rawImage.mW, 
        (int *)&rawImage.mH,   
        (int *)&rawImage.mFormat, 0);
    ASSERT_LOG(rawImage.mData != nullptr, "URL: {0}", url);

    rawImage.mLength = rawImage.mW * rawImage.mH * rawImage.mFormat;

    switch (rawImage.mFormat)
    {
    case 1: rawImage.mFormat = GL_RED; break;
    case 3: rawImage.mFormat = GL_RGB; break;
    case 4: rawImage.mFormat = GL_RGBA; break;
    }

    auto md5 = MD5(rawImage.mData, rawImage.mLength);
    _rawImageMap.insert(std::make_pair(md5.str, rawImage));
}

void RawManager::ImportProgram(const std::string & url)
{
    std::ifstream istream(url);
    ASSERT_LOG(istream, "URL: {0}", url);

    RawProgram rawProgram;
    rawProgram.mLength = file_tool::GetFileLength(istream);
    rawProgram.mData = new uchar[rawProgram.mLength];
    istream.read((char *)rawProgram.mData, rawProgram.mLength);

    auto md5 = MD5(rawProgram.mData, rawProgram.mLength);
    _rawProgramMap.insert(std::make_pair(md5.str, rawProgram));
    istream.close();
}

void RawManager::ImportMaterial(const std::string & url)
{
    std::ifstream istream(url);
    ASSERT_LOG(istream, "URL: {0}", url);
    ASSERT_LOG(sizeof(RawMaterial) == file_tool::GetFileLength(istream), "URL: {0}", url);

    RawMaterial rawMaterial;
    istream.read((char *)&rawMaterial, sizeof(RawMaterial));

    auto md5 = MD5((char *)&rawMaterial, sizeof(RawMaterial));
    _rawMaterialMap.insert(std::make_pair(md5.str, rawMaterial));
    istream.close();
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
    istream.read((char *)&rawImage, sizeof(RawImage::mW) 
                                  + sizeof(RawImage::mH) 
                                  + sizeof(RawImage::mFormat) 
                                  + sizeof(RawImage::mLength));
    rawImage.mData = new uchar[rawImage.mLength];
    istream.read((char *)rawImage.mData, sizeof(uchar) * rawImage.mLength);

    _rawImageMap.insert(std::make_pair(key, rawImage));
}

void RawManager::LoadRawProgram(std::ifstream & istream, const std::string & key)
{
    RawProgram rawProgram;
    istream.read((char *)&rawProgram, sizeof(RawProgram::mLength));
    rawProgram.mData = new uchar[rawProgram.mLength];
    istream.read((char *)rawProgram.mData, sizeof(uchar) * rawProgram.mLength);

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
