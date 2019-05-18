#include "raw_manager.h"
#include "../cfg/cfg_manager.h"
//  STB IMAGE 加载图片用
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "../third/stb_image.h"

const std::array<std::string, RawManager::kRawTypeEnum> RawManager::RAWDATA_REF = {
    {
        "raw/head.db",
        "raw/mesh.db",
        "raw/image.db",
        "raw/program.db",
        "raw/material.db",
    }
};

const std::array<std::vector<std::string>, RawManager::kImportTypeEnum> RawManager::SUFFIX_MAP = {
    { 
        { ".obg", ".fbx" }, 
        { ".png", ".jpg" }, 
        { ".program" },
        { ".material" },
    }
};

bool RawManager::Import(const std::string & url)
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
    return Import(url, type);
}

bool RawManager::Import(const std::string & url, ImportTypeEnum type)
{
    switch (type)
    {
    case RawManager::kIMPORT_MODEL: return ImportModel(url);
    case RawManager::kIMPORT_IMAGE: return ImportImage(url);
    case RawManager::kIMPORT_PROGRAM: return ImportProgram(url);
    case RawManager::kIMPORT_MATERIAL: return ImportMaterial(url);
    }
    ASSERT_LOG(false, "导入的资源格式非法!: {0}, {1}", type, url);
    return false;
}

bool RawManager::ImportModel(const std::string & url)
{
    return false;
}

bool RawManager::ImportImage(const std::string & url)
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

bool RawManager::ImportProgram(const std::string & url)
{
    return false;
}

bool RawManager::ImportMaterial(const std::string & url)
{
    return false;
}

void RawManager::LoadRawHead()
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
