#pragma once

#include "component.h"
#include "../res/mesh.h"
#include "../res/bitmap.h"
#include "../res/bitmap_cube.h"

class RenderTarget;

class Light : public Component {
public:
    class LightPool {
    public:
        static const uint ALLOC_LENGTH_STEP = 8;

    public:
        LightPool()
            : _tex2D(0), _tex3D(0)
            , _len2D(0), _len3D(0), _lenUBO(0)
        { }

        void Clear();
        uint NewUBO();
        uint NewPos2D();
        uint NewPos3D();
        const uint & GetTex2D() const { return _tex2D; }
        const uint & GetTex3D() const { return _tex3D; }
        const uint & GetUBO(uint pos) const { return _uboArray.at(pos); }

        void FreeUBO(uint pos);
        void FreePos2D(uint pos);
        void FreePos3D(uint pos);

    private:
        void AllocUBO();
        void AllocPos2D();
        void AllocPos3D();

    private:
        uint _tex2D, _tex3D;
        uint _len2D, _len3D, _lenUBO;
        std::vector<uint> _uboArray;
        std::vector<uint> _uboStock;
        std::vector<uint> _posStock2D;
        std::vector<uint> _posStock3D;
    };

public:
	enum Type {
		kDIRECT,
		kPOINT,
		kSPOT,
	};

protected:
    static uint s_VIEW_W;
    static uint s_VIEW_H;
    static LightPool s_lightPool;

private:
    static std::weak_ptr<Mesh> s_directVolmue;
    static std::weak_ptr<Mesh> s_pointVolmue;
    static std::weak_ptr<Mesh> s_spotVolmue;
    std::shared_ptr<Mesh> NewVolume();

public:
	Light(Type type)
        : _type(type), _uboPos(0)
    { 
        mSMP = _type == Type::kDIRECT? s_lightPool.NewPos2D()
             : _type == Type::kPOINT? s_lightPool.NewPos3D()
             :  s_lightPool.NewPos2D();
        _uboPos = s_lightPool.NewUBO();
        _volume = NewVolume();
    }

    virtual ~Light()
    {
        switch (_type)
        {
        case Light::kDIRECT: { s_lightPool.FreePos2D(mSMP); } break;
        case Light::kPOINT: { s_lightPool.FreePos3D(mSMP); } break;
        case Light::kSPOT: { s_lightPool.FreePos2D(mSMP); } break;
        }
        s_lightPool.FreeUBO(_uboPos);
    }

	virtual void OnAdd();
	virtual void OnDel();
	virtual void OnUpdate(float dt) { }
    virtual bool NextDrawShadow(uint count, RenderTarget * rt) = 0;

    const Type & GetType() const { return _type; }
    const Mesh & GetMesh() const { return *_volume; }
    const uint & GetUBO() const { return s_lightPool.GetUBO(_uboPos); }
    static uint GetShadowMap2D() { return s_lightPool.GetTex2D(); }
    static uint GetShadowMap3D() { return s_lightPool.GetTex3D(); }
    static float CalLightDistance(float k0, float k1, float k2, float s);

public:
    glm::uint mSMP;
    glm::mat4 mMatrix;
    glm::vec3 mAmbient;
	glm::vec3 mDiffuse;
	glm::vec3 mSpecular;
    glm::vec3 mPosition;

protected:
    //  UBO
    uint _uboPos;
    //  光源投影矩阵
    glm::mat4 _proj;
    //  光体积
    std::shared_ptr<Mesh> _volume;
private:
	Type _type;
};

class LightDirect : public Light {
public:
    //  这个结构定义仅仅起到说明作用, 该结构对应Shader里的定义
    struct UBOData {
        uint mSMP;
        glm::mat4 mMatrix;
        glm::vec3 mNormal;
        glm::vec3 mAmbient;
        glm::vec3 mDiffuse;
        glm::vec3 mSpecular;
        glm::vec3 mPosition;
    };

public:
    static uint GetUBOLength();

public:
	LightDirect(): Light(Light::kDIRECT)
	{ }

    ~LightDirect()
    { }

	void OpenShadow(const glm::vec2 & orthoX,	//	左右
					const glm::vec2 & orthoY,	//	上下
					const glm::vec2 & orthoZ);	//	前后

	virtual bool NextDrawShadow(uint count, RenderTarget * rt) override;

public:
    glm::vec3 mNormal;
};

class LightPoint : public Light {
public:
    //  这个结构定义仅仅起到说明作用, 该结构对应Shader里的定义
    struct UBOData {
        uint mSMP;
        float mFar, mNear;
        float mK0, mK1, mK2;
        glm::vec3 mAmbient;
        glm::vec3 mDiffuse;
        glm::vec3 mSpecular;
        glm::vec3 mPosition;
    };

    static uint GetUBOLength();

public:
	LightPoint(): Light(Light::kPOINT)
	{ }

    ~LightPoint()
    { }

	void OpenShadow(const float n, const float f);

    virtual bool NextDrawShadow(uint count, RenderTarget * rt) override;

public:
    float mFar, mNear;
    float mK0, mK1, mK2;
};

class LightSpot : public Light {
public:
    //  这个结构定义仅仅起到说明作用, 该结构对应Shader里的定义
    struct UBOData {
        uint mSMP;
        float mK0, mK1, mK2;
        float mInCone, mOutCone;
        glm::mat4 mMatrix;
        glm::vec3 mNormal;
        glm::vec3 mAmbient;
        glm::vec3 mDiffuse;
        glm::vec3 mSpecular;
        glm::vec3 mPosition;
    };

    static uint GetUBOLength();

public:
	LightSpot(): Light(Light::kSPOT)
	{ }

    ~LightSpot()
    { }

	void OpenShadow(const float n, const float f);

    virtual bool NextDrawShadow(uint count, RenderTarget * rt) override;

public:
	glm::vec3 mNormal;
    float mK0, mK1, mK2;
	float mOutCone, mInCone;
};