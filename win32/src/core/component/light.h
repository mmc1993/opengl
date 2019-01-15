#pragma once

#include "component.h"

class Mesh;
class Shader;

class Light : public Component {
public:
	Light();
	~Light();

	virtual void OnAdd();
	virtual void OnDel();
	virtual void OnUpdate(float dt);
	
public:
	glm::vec3 mAmbient;
	glm::vec3 mDiffuse;
	glm::vec3 mSpecular;

	Mesh * mMesh;
	Shader * mShader;
};