#pragma once

#include "component.h"

class Shader;
class Bitmap;
class BitmapCube;
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
	virtual void DrawShadow() = 0;
	LightType GetType() { return _type; }

public:
	bool mIsDraw;
	glm::vec3 mAmbient;
	glm::vec3 mDiffuse;
	glm::vec3 mSpecular;

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
	LightDirect()
		: Light(Light::kDIRECT)
		, _shadowTex(nullptr)
		, _shadowRT(nullptr)
	{ }

	~LightDirect();

	void OpenShadow(const std::uint32_t depthW, const std::uint32_t depthH,
					const float orthoXMin, const float orthoXMax,
					const float orthoYMin, const float orthoYMax,
					const float orthoZMin, const float orthoZMax,
					const glm::vec3 &up);
	void HideShadow();
	const glm::mat4 & GetShadowMat() const;
	const Bitmap * GetShadowTex() const;
	virtual void DrawShadow() override;

public:
	glm::vec3 mNormal;

private:
	RenderTarget * _shadowRT;
	std::uint32_t _depthW;
	std::uint32_t _depthH;
	glm::mat4 _shadowMat;
	Bitmap * _shadowTex;
	glm::vec2 _orthoX;
	glm::vec2 _orthoY;
	glm::vec2 _orthoZ;
	glm::vec3 _up;

};

class LightPoint : public Light {
public:
	LightPoint()
		: Light(Light::kPOINT)
		, _shadowTex(nullptr)
		, _shadowRT(nullptr)
	{ }

	~LightPoint()
	{ }

	void OpenShadow(const std::uint32_t depthW,
					const std::uint32_t depthH, 
					const float n, const float f);
	void HideShadow();
	const BitmapCube * GetShadowTex() const;
	const glm::mat4 & GetShadowMat() const;
	virtual void DrawShadow() override;

public:
	float mK0, mK1, mK2;

private:
	void DrawShadow(size_t idx, const glm::mat4 & view);

private:
	RenderTarget * _shadowRT;
	BitmapCube * _shadowTex;
	std::uint32_t _depthW;
	std::uint32_t _depthH;
	glm::mat4 _proj;
	float _n, _f;
};

class LightSpot : public Light {
public:
	LightSpot()
		: Light(Light::kSPOT)
		, _shadowTex(nullptr)
		, _shadowRT(nullptr)
	{ }

	~LightSpot()
	{ }

	void OpenShadow(const std::uint32_t depthW, 
					const std::uint32_t depthH,
					const float n, const float f,
					const glm::vec3 & up);
	void HideShadow();
	const glm::mat4 & GetShadowMat() const;
	const Bitmap * GetShadowTex() const;
	virtual void DrawShadow() override;

public:
	glm::vec3 mNormal;
	float mK0, mK1, mK2;
	float mOutCone, mInCone;

private:
	RenderTarget * _shadowRT;
	glm::mat4 _shadowMat;
	Bitmap * _shadowTex;
	std::uint32_t _depthW;
	std::uint32_t _depthH;
	glm::vec3 _up;
	float _n, _f;
};