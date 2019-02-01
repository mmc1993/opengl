#pragma once

#include "component.h"

class Mesh;
class Shader;
class RenderTarget;

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
	virtual RenderTarget * DrawShadow(bool onlyGet) = 0;
	const glm::mat4 & GetShadowMatrix() const { return _matrixVP; }
	void HideShadow();

public:
	bool mIsDraw;
	glm::vec3 mAmbient;
	glm::vec3 mDiffuse;
	glm::vec3 mSpecular;

protected:
	RenderTarget * _shadowRT;
	glm::mat4 _matrixVP;

private:
	GLuint _vbo;
	GLuint _ebo;
	GLuint _vao;
	Shader * _shader;

private:
	LightType _type;
};

class LightDirect : public Light {
public:
	LightDirect(): Light(Light::kDIRECT)
	{ }

	~LightDirect()
	{ }

	void OpenShadow(const std::uint32_t depthW, const std::uint32_t depthH,
					const float orthoXMin, const float orthoXMax,
					const float orthoYMin, const float orthoYMax,
					const float orthoZMin, const float orthoZMax,
					const glm::vec3 &up);
	virtual RenderTarget * DrawShadow(bool onlyGet) override;

public:
	glm::vec3 mNormal;

private:
	std::uint32_t _depthW;
	std::uint32_t _depthH;
	glm::vec2 _orthoX;
	glm::vec2 _orthoY;
	glm::vec2 _orthoZ;
	glm::vec3 _up;
};

class LightPoint : public Light {
public:
	LightPoint(): Light(Light::kPOINT)
	{ }

	~LightPoint()
	{ }

	void OpenShadow(const std::uint32_t depthW,
					const std::uint32_t depthH, 
					const float n, const float f);

	virtual RenderTarget * DrawShadow(bool onlyGet) override;

public:
	float mK0, mK1, mK2;

private:
	std::uint32_t _depthW;
	std::uint32_t _depthH;
	float _n, _f;
};

class LightSpot : public Light {
public:
	LightSpot(): Light(Light::kSPOT)
	{ }

	~LightSpot()
	{ }

	void OpenShadow(const std::uint32_t depthW, 
					const std::uint32_t depthH,
					const float n, const float f,
					const glm::vec3 & up);

	virtual RenderTarget * DrawShadow(bool onlyGet) override;

public:
	glm::vec3 mNormal;
	float mK0, mK1, mK2;
	float mOutCone, mInCone;

private:
	std::uint32_t _depthW;
	std::uint32_t _depthH;
	glm::vec3 _up;
	float _n, _f;
};