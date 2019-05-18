#pragma once

#include "../include.h"

class RawManager {
public:
    enum ImportTypeEnum {
        kIMPORT_MODEL,
        kIMPORT_IMAGE,
        kIMPORT_PROGRAM,
        kIMPORT_MATERIAL,
        kImportTypeEnum,
    };

    enum RawTypeEnum {
        kRAW_HEAD,
        kRAW_MESH,
        kRAW_IMAGE,
        kRAW_PROGRAM,
        kRAW_MATERIAL,
        kRawTypeEnum,
    };

    struct RawImage {
        uint mW, mH;
        uint mFormat;
        uint mLength;
        uchar *mData;
    };

    //  原始数据头部信息
    struct RawHead {
        struct Head {
            uint mMeshOffset;
            uint mMeshLength;
            uint mImageOffset;
            uint mImageLength;
            uint mProgramOffset;
            uint mProgramLength;
            uint mMaterialOffset;
            uint mMaterialLength;
        };

        struct Info {
            char mMD5[16];
            uint mOffset;
            uint mLength;
        };
        std::vector<Info> mMeshList;
        std::vector<Info> mImageList;
        std::vector<Info> mProgramList;
        std::vector<Info> mMaterialList;
    };

    static const std::array<std::string, kRawTypeEnum> RAWDATA_REF;

    static const std::array<std::vector<std::string>, kImportTypeEnum> SUFFIX_MAP;

public:
    bool Import(const std::string & url);

private:
    bool Import(const std::string & url, ImportTypeEnum type);
    bool ImportModel(const std::string & url);
    bool ImportImage(const std::string & url);
    bool ImportProgram(const std::string & url);
    bool ImportMaterial(const std::string & url);

    void LoadRawHead();
    void SaveRawData();

private:
    RawHead _rawHead;
    std::map<std::string, RawImage> _rawImageMap;
    std::ifstream _istreams[kRawTypeEnum];

};