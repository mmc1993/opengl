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

    struct RawMesh {
        uint mIndexLength;
        uint mVertexLength;
        uint * mIndexs;
        float * mVertexs;
    };

    struct RawImage {
        uint mW, mH;
        uint mFormat;
        uint mLength;
        uchar *mData;
    };

    struct RawProgram {
        uint mLength;
        uchar *mData;
    };

    struct RawMaterial {
        struct Texture {
            char mName[16];
            char mImage[16];
        };
        uint mShininess;
        char mMesh[16];
        char mProgram[16];
        Texture mTextures[8];
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
            uint mByteOffset;
            uint mByteLength;
            Info(const char * md5, uint offset, uint length)
                : mByteOffset(offset)
                , mByteLength(length)
            { 
                memcpy(mMD5, md5, sizeof(mMD5));
            }
            Info() { }
        };
        std::vector<Info> mMeshList;
        std::vector<Info> mImageList;
        std::vector<Info> mProgramList;
        std::vector<Info> mMaterialList;
    };

    static const std::array<std::string, kRawTypeEnum> RAWDATA_REF;

    static const std::array<std::vector<std::string>, kImportTypeEnum> SUFFIX_MAP;

public:
    void BegImport();
    void EndImport();
    void Import(const std::string & url);

private:
    void Import(const std::string & url, ImportTypeEnum type);
    void ImportModel(const std::string & url);
    void ImportImage(const std::string & url);
    void ImportProgram(const std::string & url);
    void ImportMaterial(const std::string & url);

    void InitRawHead();
private:
    RawHead _rawHead;
    std::ifstream _istreams[kRawTypeEnum];
    std::map<std::string, RawMesh> _rawMeshMap;
    std::map<std::string, RawImage> _rawImageMap;
    std::map<std::string, RawProgram> _rawProgramMap;
    std::map<std::string, RawMaterial> _rawMaterialMap;
};