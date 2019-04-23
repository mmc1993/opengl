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
            : _len2D(0)
            , _len3D(0)
            , _tex2D(0)
            , _tex3D(0)
        {}
        void Clear();
        uint GetTex2D();
        uint GetTex3D();
        uint GetPos2D();
        uint GetPos3D();
        void FreePos2D(uint id);
        void FreePos3D(uint id);

    private:
        void AllocPos2D();
        void AllocPos3D();

    private:
        std::vector<uint> _posStock2D;
        std::vector<uint> _posStock3D;
        uint _len2D;
        uint _len3D;
        uint _tex2D;
        uint _tex3D;
    };

public:
    static void Init(uint texW, uint texH)
    {
        s_VIEW_W = texW;
        s_VIEW_H = texH;
    }

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
        _smp = _type == Type::kDIRECT? s_shadowMapPool.GetPos2D()
             : _type == Type::kPOINT? s_shadowMapPool.GetPos3D()
             : s_shadowMapPool.GetPos2D();
    }

    virtual ~Light()
    {
        switch (_type)
        {
        case Light::kDIRECT: { s_shadowMapPool.FreePos2D(_smp); } break;
        case Light::kPOINT: { s_shadowMapPool.FreePos3D(_smp); } break;
        case Light::kSPOT: { s_shadowMapPool.FreePos2D(_smp); } break;
        }
        glDeleteBuffers(1, &_ubo);
    }

	virtual void OnAdd();
	virtual void OnDel();
	virtual void OnUpdate(float dt) { }
    virtual bool NextDrawShadow(uint count, RenderTarget * rt) = 0;

    static uint GetShadowMap2D() { return s_shadowMapPool.GetTex2D(); }
    static uint GetShadowMap3D() { return s_shadowMapPool.GetTex3D(); }
    const glm::vec3 & GetWorldPos() const { return _pos; }
    const glm::mat4 & GetMatrix() const {return _matrix; }
    const uint & GetUniformBlock() const { return _ubo; }
    const uint & GetShadowMapPos() const { return _smp; }
    const Type & GetType() const { return _type; }

public:
	glm::vec3 mAmbient;
	glm::vec3 mDiffuse;
	glm::vec3 mSpecular;

protected:
    //  UBO
    uint _ubo;
    //  Shadow Map Pos
    uint _smp;
    //  光源世界坐标
    glm::vec3 _pos;
    //  光源投影矩阵
    glm::mat4 _proj;
    //  光源视图矩阵
    glm::mat4 _matrix;
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
        float mK0, mK1, mK2;
        glm::vec3 mAmbient;
        glm::vec3 mDiffuse;
        glm::vec3 mSpecular;
        glm::vec3 mPosition;
    };

public:
	LightPoint(): Light(Light::kPOINT)
	{ }

    ~LightPoint()
    { }

	void OpenShadow(const float n, const float f);

    virtual bool NextDrawShadow(uint count, RenderTarget * rt) override;

public:
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