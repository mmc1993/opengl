#pragma once

#include "component.h"

#include "../raw/gl_mesh.h"
#include "../raw/gl_program.h"
#include "../raw/gl_material.h"
#include "../raw/gl_texture2d.h"

class RenderTarget;

class Light : public Component {
public:
	enum TypeEnum {
		kDIRECT,
		kPOINT,
		kSPOT,
	};

private:
    std::shared_ptr<GLMesh> NewVolume();

public:
    Light(TypeEnum type);
    virtual ~Light() {  }

    virtual void OnAdd() {}
    virtual void OnDel() {}
    virtual void OnUpdate(float dt);
    virtual bool NextDrawShadow(uint count, uint shadow, RenderTarget * rt) = 0;

    uint      GetUBO() const { return _ubo; }
    TypeEnum GetType() const { return _type; }
    static float CalLightDistance(float k0, float k1, float k2, float s);

public:
    glm::mat4 mMatrix;
    glm::vec3 mAmbient;
	glm::vec3 mDiffuse;
	glm::vec3 mSpecular;
    glm::vec3 mPosition;

protected:
    uint        _ubo;
    glm::mat4   _proj;
    GLProgram   * _program;
    GLMesh      * _volume;
private:
	TypeEnum _type;
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
    static uint GetUBOLength();

public:
	LightDirect(): Light(Light::kDIRECT)
	{ }

    ~LightDirect()
    { }

    virtual void OnUpdate(float dt) override;

	void OpenShadow(const glm::vec2 & orthoX,	//	左右
					const glm::vec2 & orthoY,	//	上下
					const glm::vec2 & orthoZ);	//	前后

	virtual bool NextDrawShadow(uint count, uint shadow, RenderTarget * rt) override;

public:
    glm::vec3 mNormal;

private:
    glm::mat4 _view;
};

class LightPoint : public Light {
public:
    //  这个结构定义仅仅起到说明作用, 该结构对应Shader里的定义
    struct UBOData {
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

    virtual void OnUpdate(float dt) override;

	void OpenShadow(const float n, const float f);

    virtual bool NextDrawShadow(uint count, uint shadow, RenderTarget * rt) override;

public:
    float mFar, mNear;
    float mK0, mK1, mK2;
};

class LightSpot : public Light {
public:
    //  这个结构定义仅仅起到说明作用, 该结构对应Shader里的定义
    struct UBOData {
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

    virtual void OnUpdate(float dt) override;

	void OpenShadow(const float n, const float f);

    virtual bool NextDrawShadow(uint count, uint shadow, RenderTarget * rt) override;

public:
	glm::vec3 mNormal;
    float mK0, mK1, mK2;
	float mOutCone, mInCone;

private:
    glm::mat4 _view;
};