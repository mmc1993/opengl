#pragma once

#include "../include.h"
#include "gl_mesh.h"
#include "gl_program.h"
#include "gl_material.h"
#include "gl_texture2d.h"

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
        kRAW_MESH,
        kRAW_IMAGE,
        kRAW_PROGRAM,
        kRAW_MATERIAL,
        kRawTypeEnum,
    };

    class Manifest {
    public:
        struct Info {
            char mName[RAW_NAME_LEN];
            std::string mURL;

            Info() { }

            Info(const char * name, const std::string & url): mURL(url)
            {
                std::copy(name, name + RAW_NAME_LEN, mName);
            }
        };

        struct Slot {
            char mName[RAW_NAME_LEN];
            uint mByteOffset;
            uint mByteLength;
            RawTypeEnum mType;

            Slot() { }

            Slot(const char * name, uint byteOffset, uint byteLength, RawTypeEnum type)
                : mByteOffset(byteOffset)
                , mByteLength(byteLength)
                , mType(type)
            {
                std::copy(name, name + RAW_NAME_LEN, mName);
            }

            bool operator==(const std::string & name) const
            {
                return name == mName;
            }
        };
        std::vector<Info> mInfos;
        std::vector<Slot> mSlots;
    };

    class Raw {
    protected:
        Raw() {}
    public:
        virtual ~Raw() {}
        virtual void Serialize(std::ofstream & os) = 0;
        virtual void Deserialize(std::ifstream & is) = 0;
    };

    class RawMesh : public Raw {
    public:
        uint mIndexLength;
        uint mVertexLength;
        uint           * mIndexs;
        GLMesh::Vertex * mVertexs;

        RawMesh();
        ~RawMesh();
        virtual void Serialize(std::ofstream & os) override;
        virtual void Deserialize(std::ifstream & is) override;
    };

    class RawImage : public Raw {
    public:
        uint mW, mH;
        uint mFormat;
        uchar *mData;
        uint mByteLength;

        RawImage();
        ~RawImage();
        virtual void Serialize(std::ofstream & os) override;
        virtual void Deserialize(std::ifstream & is) override;
    };

    class RawProgram : public Raw {
    public:
        uchar * mData;
        uint mPassLength;
        uint mVSByteLength;
        uint mGSByteLength;
        uint mFSByteLength;

        RawProgram();
        ~RawProgram();
        virtual void Serialize(std::ofstream & os) override;
        virtual void Deserialize(std::ifstream & is) override;
    };

    class RawMaterial : public Raw {
    public:
        struct Texture {
            char mName[RAW_NAME_LEN];
            char mTexture[RAW_NAME_LEN];
        };
        uint mShininess;
        char mMesh[RAW_NAME_LEN];
        char mProgram[RAW_NAME_LEN];
        Texture mTextures[MTLTEX2D_LEN];

        RawMaterial();
        ~RawMaterial();
        virtual void Serialize(std::ofstream & os) override;
        virtual void Deserialize(std::ifstream & is) override;
    };

    static const std::string MANIFEST_SLOT_URL;
    static const std::string MANIFEST_INFO_URL;
    static const std::array<std::string, kRawTypeEnum> RAWDATA_URL;
    static const std::array<std::vector<std::string>, kImportTypeEnum> SUFFIX_MAP;

public:
    void Init();
    void BegImport(bool clear = false);
    void EndImport();
    void Import(const std::string & url);

    //  将原始数据加载到内存
    Raw * LoadRaw(const std::string & name);
    //  将原始数据从内存卸载
    void FreeRaw(const std::string & name);

    //  通过原始数据构造对象
    template <class T>
    T * LoadRes(const std::string & name)
    {
        auto it = _resObjectMap.find(name);
        if (it != _resObjectMap.end())
        {
            ASSERT_LOG(dynamic_cast<T *>(it->second) != nullptr, "Res Type Not Match! {0}, {1}", name, typeid(T).name(), typeid(*it->second).name());
            return reinterpret_cast<T *>(it->second);
        }
        auto res = (GLRes *)nullptr;
        auto rawIt = std::find(_manifest.mSlots.begin(), 
                               _manifest.mSlots.end(), name);
        switch (rawIt->mType)
        {
        case kRAW_MESH: res = LoadResMesh(name); break;
        case kRAW_IMAGE: res = LoadResImage(name); break;
        case kRAW_PROGRAM: res = LoadResProgram(name); break;
        case kRAW_MATERIAL: res = LoadResMaterial(name); break;
        }
        ASSERT_LOG(res != nullptr, "Not Found Res. {0}, {1}", name, typeid(T).name());
        ASSERT_LOG(dynamic_cast<T *>(res) != nullptr, "Res Type Not Match. {0}, {1}, {2}", name, typeid(T).name(), typeid(*res).name());
        return reinterpret_cast<T *>(res);
    }

    //  销毁对象, 保留原始数据
    void FreeRes(const std::string & name);

private:
    void ImportModel(const std::string & url);
    void ImportImage(const std::string & url);
    void ImportProgram(const std::string & url);
    void ImportMaterial(const std::string & url);

    //  从原始数据创建资源
    GLRes * LoadResMesh(const std::string & name);
    GLRes * LoadResImage(const std::string & name);
    GLRes * LoadResProgram(const std::string & name);
    GLRes * LoadResMaterial(const std::string & name);

    //  为数据生成名字
    std::string BuildName(const uchar * data, const uint len);

private:
    //  资源对象
    std::map<std::string, GLRes *> _resObjectMap;
    std::map<std::string, Raw *> _rawObjectMap;
    Manifest _manifest;
};
