#pragma once

#include "component.h"
#include "../asset/bitmap.h"
#include "../asset/bitmap_cube.h"

class RenderTarget;

class Light : public Component {
public:
    class ShadowMapPool {
    public:
        static const uint s_LEN_STEP = 0x8;

    public:
        ShadowMapPool()
            : _len2D(0) , _len3D(0)
            , _tex2D(0) , _tex3D(0)
        {}
        void Clear();
        uint GetTex2D();
        uint GetTex3D();
        uint NewPos2D();
        uint NewPos3D();
        void FreePos2D(uint id);
        void FreePos3D(uint id);

    private:
        void AllocPos2D();
        void AllocPos3D();

    private:
        uint _len2D, _len3D;
        uint _tex2D, _tex3D;
        std::vector<uint> _posStock2D;
        std::vector<uint> _posStock3D;
    };

protected:
    static uint s_VIEW_W;
    static uint s_VIEW_H;
    static ShadowMapPool s_shadowMapPool;

public:
	enum Type {
		kDIRECT,
		kPOINT,
		kSPOT,
	};

public:
	Light(Type type)
        : _type(type), _ubo(0)
    { 
        mSMP = _type == Type::kDIRECT? s_shadowMapPool.NewPos2D()
             : _type == Type::kPOINT? s_shadowMapPool.NewPos3D()
             : s_shadowMapPool.NewPos2D();
        glGenBuffers(1, &_ubo);
    }

    virtual ~Light()
    {
        switch (_type)
        {
        case Light::kDIRECT: { s_shadowMapPool.FreePos2D(mSMP); } break;
        case Light::kPOINT: { s_shadowMapPool.FreePos3D(mSMP); } break;
        case Light::kSPOT: { s_shadowMapPool.FreePos2D(mSMP); } break;
        }
        glDeleteBuffers(1, &_ubo);
    }

	virtual void OnAdd();
	virtual void OnDel();
	virtual void OnUpdate(float dt) { }
    virtual bool NextDrawShadow(uint count, RenderTarget * rt) = 0;

    static uint GetShadowMap2D() { return s_shadowMapPool.GetTex2D(); }
    static uint GetShadowMap3D() { return s_shadowMapPool.GetTex3D(); }
    const Type & GetType() const { return _type; }
    const uint & GetUBO() const { return _ubo; }

public:
    glm::uint mSMP;
    glm::mat4 mMatrix;
    glm::vec3 mAmbient;
	glm::vec3 mDiffuse;
	glm::vec3 mSpecular;
    glm::vec3 mPosition;

protected:
    //  UBO
    uint _ubo;
    //  光源投影矩阵
    glm::mat4 _proj;
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
        float mNear, mFar;
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
    float mNear, mFar;
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