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

    class ManifestSlot {
    public:
        uint mByteOffset;
        uint mByteLength;
        std::string mName;
        RawTypeEnum mType;

        ManifestSlot() = default;

        ManifestSlot(
            uint byteOffset, 
            uint byteLength, 
            const RawTypeEnum & type,
            const std::string & name)
            : mByteOffset(byteOffset)
            , mByteLength(byteLength)
            , mType(type),mName(name)
        { }

        bool operator==(const std::string & name) const
        {
            return name == mName;
        }
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
        std::vector<uint>           mIndexs;
        std::vector<GLMesh::Vertex> mVertexs;
        
        virtual void Serialize(std::ofstream & os) override;
        virtual void Deserialize(std::ifstream & is) override;
    };

    class RawImage : public Raw {
    public:
        uint mW, mH;
        uint mFormat;
        std::string mData;

        virtual void Serialize(std::ofstream & os) override;
        virtual void Deserialize(std::ifstream & is) override;
    };

    class RawProgram : public Raw {
    public:
        std::vector<GLProgram::PassAttr> mAttrs;
        std::string mVSBuffer;
        std::string mGSBuffer;
        std::string mFSBuffer;

        virtual void Serialize(std::ofstream & os) override;
        virtual void Deserialize(std::ifstream & is) override;
    };

    class RawMaterial : public Raw {
    public:
        struct Texture {
            std::string mDsec;
            std::string mName;
        };
        uint mShininess;
        std::string mMesh;
        std::string mProgram;
        std::vector<Texture> mTexture2Ds;

        virtual void Serialize(std::ofstream & os) override;
        virtual void Deserialize(std::ifstream & is) override;
    };

    static const std::string MANIFEST_SLOT_URL;
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
        auto rawIt = std::find(_manifest.begin(), _manifest.end(), name);
        switch (rawIt->mType)
        {
        case kRAW_MESH: res = LoadResMesh(name); break;
        case kRAW_IMAGE: res = LoadResImage(name); break;
        case kRAW_PROGRAM: res = LoadResProgram(name); break;
        case kRAW_MATERIAL: res = LoadResMaterial(name); break;
        }
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

private:
    //  资源对象
    std::map<std::string, GLRes *> _resObjectMap;
    std::map<std::string, Raw *> _rawObjectMap;
    std::vector<ManifestSlot> _manifest;
};
