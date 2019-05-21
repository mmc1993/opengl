#pragma once

#include "../include.h"

class Res;

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
        uint *  mIndexs;
        float * mVertexs;
    };

    struct RawImage {
        uint mW, mH;
        uint mFormat;
        uint mByteLength;
        uchar *mData;
    };

    struct RawProgram {
        struct PassAttr {
            //  面剔除
            int     vCullFace;
            //  混合
            int     vBlendSrc;
            int     vBlendDst;
            //  深度测试
            bool    bDepthTest;         //  开启深度测试
            bool    bDepthWrite;        //  开启深度写入
            //  模板测试
            int     vStencilOpFail;		//	模板测试失败
            int     vStencilOpZFail;	//	深度测试失败
            int     vStencilOpZPass;	//	深度测试通过
            int		vStencilFunc;		//	模板测试函数
            int     vStencilMask;       //  模板测试值
            int     vStencilRef;        //  模板测试值
            //  渲染
            uint    vDrawType;			//	draw类型
            uint    vRenderType;        //  渲染类型
            uint    vRenderQueue;       //  渲染通道
        };
        uchar * mData;
        uint mPassLength;
        uint mVSByteLength;
        uint mGSByteLength;
        uint mFSByteLength;
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
            uint mMeshLength;
            uint mImageLength;
            uint mProgramLength;
            uint mMaterialLength;
        };

        struct Info {
            char mMD5[16];
            uint mByteOffset;
            uint mByteLength;
            Info(const char * md5, uint offset, uint length)
                : mByteOffset(offset), mByteLength(length)
            { 
                memcpy(mMD5, md5, sizeof(mMD5));
            }

            bool operator==(const std::string & md5) const
            {
                return md5 == mMD5;
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
    void Init();
    void BegImport();
    void EndImport();
    void Import(const std::string & url);

    //  将原始数据加载到内存
    bool LoadRaw(const std::string & key);
    bool LoadRaw(const std::string & key, RawTypeEnum type);
    //  将原始数据从内存卸载
    void FreeRaw(const std::string & key);
    bool FreeRaw(const std::string & key, RawTypeEnum type);

    //  通过原始数据构造对象
    template <class T>
    T & LoadRes(const std::string & key) const;
    //  销毁对象, 保留原始数据
    void FreeRes(const Res * res);
    void FreeRes(const std::string & key);

private:
    void Import(const std::string & url, ImportTypeEnum type);
    void ImportModel(const std::string & url);
    void ImportImage(const std::string & url);
    void ImportProgram(const std::string & url);
    void ImportMaterial(const std::string & url);

    //  加载原始数据到内存
    void LoadRawMesh(std::ifstream & istream, const std::string & key);
    void LoadRawImage(std::ifstream & istream, const std::string & key);
    void LoadRawProgram(std::ifstream & istream, const std::string & key);
    void LoadRawMaterial(std::ifstream & istream, const std::string & key);

    void ClearRawData();

private:
    RawHead _rawHead;
    std::map<std::string, Res *> _resObjectMap;
    std::map<std::string, RawMesh> _rawMeshMap;
    std::map<std::string, RawImage> _rawImageMap;
    std::map<std::string, RawProgram> _rawProgramMap;
    std::map<std::string, RawMaterial> _rawMaterialMap;
};