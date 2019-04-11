#pragma once

#include "component.h"
#include "../asset/bitmap.h"
#include "../asset/bitmap_cube.h"

class RenderTarget;

class Light : public Component {
public:
    class TexPool {
    public:
        void Clear();
        uint GetTexture2D();
        uint GetTexture3D();
        uint GetTexOrder2D();
        uint GetTexOrder3D();
        void FreeTexOrder2D(uint id);
        void FreeTexOrder3D(uint id);

    private:
        void AllocTexOrder2D();
        void AllocTexOrder3D();

    private:
        std::vector<uint> _texOrder2Ds;
        std::vector<uint> _texOrder3Ds;
        size_t _len2D;
        size_t _len3D;
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
    static TexPool s_texPool;

public:
	enum Type {
		kDIRECT,
		kPOINT,
		kSPOT,
	};

public:
	Light(Type type)
        : _type(type), _uboID(0)
    { 
        _texOrder = _type == Type::kDIRECT? s_texPool.GetTexOrder2D()
                  : _type == Type::kPOINT? s_texPool.GetTexOrder3D()
                  : s_texPool.GetTexOrder2D();
    }

    virtual ~Light()
    {
        switch (_type)
        {
        case Light::kDIRECT: { s_texPool.FreeTexOrder2D(_texOrder); } break;
        case Light::kPOINT: { s_texPool.FreeTexOrder3D(_texOrder); } break;
        case Light::kSPOT: { s_texPool.FreeTexOrder2D(_texOrder); } break;
        }
        glDeleteBuffers(1, &_uboID);
    }

	virtual void OnAdd();
	virtual void OnDel();
	virtual void OnUpdate(float dt) { }
    virtual bool NextDrawShadow(size_t count, RenderTarget * rt) = 0;

    uint GetTex2D() const { return s_texPool.GetTexture2D(); }
    uint GetTex3D() const { return s_texPool.GetTexture3D(); }
    uint GetUBOID() const { return _uboID; }
    Type GetType() const { return _type; }

public:
	glm::vec3 mAmbient;
	glm::vec3 mDiffuse;
	glm::vec3 mSpecular;

protected:
    //  UBO
    uint _uboID;
    //  Tex序号
    uint _texOrder;
private:
	Type _type;

};

class LightDirect : public Light {
public:
    //  这个结构定义仅仅起到说明作用, 该结构对应Shader里的定义
    struct UBOData {
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

	virtual bool NextDrawShadow(size_t count, RenderTarget * rt) override;

public:
    glm::vec3 mNormal;

private:
	glm::mat4 _proj;
};

class LightPoint : public Light {
public:
    //  这个结构定义仅仅起到说明作用, 该结构对应Shader里的定义
    struct UBOData {
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

    virtual bool NextDrawShadow(size_t count, RenderTarget * rt) override;

public:
	float mK0, mK1, mK2;

private:
    glm::mat4 _proj;
    glm::vec3 _pos;
};

class LightSpot : public Light {
public:
    //  这个结构定义仅仅起到说明作用, 该结构对应Shader里的定义
    struct UBOData {
        float mK0, mK1, mK2;
        float mInCone, mOutCone;
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

    virtual bool NextDrawShadow(size_t count, RenderTarget * rt) override;

public:
	glm::vec3 mNormal;
    float mK0, mK1, mK2;
	float mOutCone, mInCone;

private:
    glm::mat4 _proj;
    glm::vec3 _pos;
};