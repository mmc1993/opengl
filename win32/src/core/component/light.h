#pragma once

#include "component.h"

class Mesh;
class Shader;

class Light : public Component {
public:
	enum LightType {
		kDIRECT,
		kPOINT,
		kSPOT,
	};
public:
	Light(LightType type);
	virtual ~Light();

	virtual void OnAdd();
	virtual void OnDel();
	virtual void OnUpdate(float dt);
	LightType GetType() { return _type; }

public:
	glm::vec3 mAmbient;
	glm::vec3 mDiffuse;
	glm::vec3 mSpecular;

	Mesh * mMesh;
	Shader * mShader;

private:
	LightType _type;
};

class LightDirect : public Light {
public:
	LightDirect(): Light(Light::kDIRECT)
	{ }

	~LightDirect()
	{ }

public:
	glm::vec3 mNormal;
};

class LightPoint : public Light {
public:
	LightPoint(): Light(Light::kPOINT)
	{ }

	~LightPoint()
	{ }

public:
	float mK0, mK1, mK2;
};

class LightSpot : public Light {
public:
	LightSpot(): Light(Light::kSPOT)
	{ }

	~LightSpot()
	{ }

public:
	glm::vec3 mNormal;
	float mK0, mK1, mK2;
	float mOutCone, mInCone;
};