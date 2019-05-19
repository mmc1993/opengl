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

void RawManager::BegImport()
{
    _rawMeshMap.clear();
    _rawImageMap.clear();
    _rawProgramMap.clear();
    _rawMaterialMap.clear();

    _rawHead.mMeshList.clear();
    _rawHead.mImageList.clear();
    _rawHead.mProgramList.clear();
    _rawHead.mMaterialList.clear();

    _istreams[kRAW_HEAD].close();
    _istreams[kRAW_MESH].close();
    _istreams[kRAW_IMAGE].close();
    _istreams[kRAW_PROGRAM].close();
    _istreams[kRAW_MATERIAL].close();
}

void RawManager::EndImport()
{
    std::ofstream ostream;
    //  写入网格
    ostream.open(RAWDATA_REF[kRAW_MESH], std::ios::out | std::ios::binary);
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
    ostream.open(RAWDATA_REF[kRAW_IMAGE], std::ios::out | std::ios::binary);
    ASSERT_LOG(ostream, "导入Image失败. {0}", RAWDATA_REF[kRAW_IMAGE]);
    for (const auto & pair : _rawImageMap)
    {
        auto byteOffset = ostream.tellp();
        ostream.write((const char *)&pair.second.mW, sizeof(RawImage::mW));
        ostream.write((const char *)&pair.second.mH, sizeof(RawImage::mH));
        ostream.write((const char *)&pair.second.mLength, sizeof(RawImage::mLength));
        ostream.write((const char *)pair.second.mData, pair.second.mLength * sizeof(uchar));
        auto byteLength = ostream.tellp() - byteOffset;
        ASSERT_LOG(byteLength == sizeof(pair.second.mW)
                               + sizeof(pair.second.mH)
                               + sizeof(pair.second.mLength)
                               + sizeof(uchar) * pair.second.mLength, "数据长度不一致");
        _rawHead.mImageList.emplace_back(pair.first.c_str(), (uint)byteOffset, (uint)byteLength);
    }
    ostream.close();

    //  写入程序
    ostream.open(RAWDATA_REF[kRAW_PROGRAM], std::ios::out | std::ios::binary);
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
    ostream.open(RAWDATA_REF[kRAW_MATERIAL], std::ios::out | std::ios::binary);
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
    ostream.open(RAWDATA_REF[kRAW_HEAD], std::ios::out | std::ios::binary);
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

void RawManager::InitRawHead()
{
    //  头部信息
    _istreams[kRAW_HEAD].open(RAWDATA_REF[kRAW_HEAD], std::ios::in | std::ios::binary);
    ASSERT_LOG(_istreams[kRAW_HEAD], "读取原始数据失败!: {0}", RAWDATA_REF[kRAW_HEAD]);

    RawHead::Head head;
    _istreams[kRAW_HEAD].read((char *)&head, sizeof(RawHead::Head));
    
    _rawHead.mMeshList.resize(head.mMeshLength * sizeof(RawHead::Info));
    _istreams[kRAW_HEAD].read((char *)_rawHead.mMeshList.data(), head.mMeshLength * sizeof(RawHead::Info));

    _rawHead.mImageList.resize(head.mImageLength * sizeof(RawHead::Info));
    _istreams[kRAW_HEAD].read((char *)_rawHead.mImageList.data(), head.mImageLength * sizeof(RawHead::Info));

    _rawHead.mProgramList.resize(head.mProgramLength * sizeof(RawHead::Info));
    _istreams[kRAW_HEAD].read((char *)_rawHead.mProgramList.data(), head.mProgramLength * sizeof(RawHead::Info));

    _rawHead.mMaterialList.resize(head.mMaterialLength * sizeof(RawHead::Info));
    _istreams[kRAW_HEAD].read((char *)_rawHead.mMaterialList.data(), head.mMaterialLength * sizeof(RawHead::Info));

    _istreams[kRAW_HEAD].close();

    //  mesh
    _istreams[kRAW_MESH].open(RAWDATA_REF[kRAW_MESH], std::ios::in | std::ios::binary);
    ASSERT_LOG(_istreams[kRAW_MESH], "读取原始数据失败!: {0}", RAWDATA_REF[kRAW_MESH]);

    //  image
    _istreams[kRAW_IMAGE].open(RAWDATA_REF[kRAW_IMAGE], std::ios::in | std::ios::binary);
    ASSERT_LOG(_istreams[kRAW_IMAGE], "读取原始数据失败!: {0}", RAWDATA_REF[kRAW_IMAGE]);

    //  program
    _istreams[kRAW_PROGRAM].open(RAWDATA_REF[kRAW_PROGRAM], std::ios::in | std::ios::binary);
    ASSERT_LOG(_istreams[kRAW_PROGRAM], "读取原始数据失败!: {0}", RAWDATA_REF[kRAW_PROGRAM]);

    //  material
    _istreams[kRAW_MATERIAL].open(RAWDATA_REF[kRAW_MATERIAL], std::ios::in | std::ios::binary);
    ASSERT_LOG(_istreams[kRAW_MATERIAL], "读取原始数据失败!: {0}", RAWDATA_REF[kRAW_MATERIAL]);
}
