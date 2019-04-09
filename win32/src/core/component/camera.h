#pragma once

#include "component.h"

class Camera: public Component {
public:
	enum Type {
		//	Õý½»
		kORTHOGONAL,
		//	Í¸ÊÓ
		kPERSPECTIVE,
	};

	union Info
	{
		struct Ortho {
			float l;
			float r;
			float t;
			float b;
			float n;
			float f;
		} mOrtho;
		struct Persp {
			float fov;
			float w;
			float h;
			float n;
			float f;
		} mPersp;
	};

public:
    Camera();
    ~Camera();
	void InitPerspective(float fov, float w, float h, float n, float f);
	void InitOrthogonal(float l, float r, float t, float b, float n, float f);
    void LookAt(const glm::vec3 & pos, 
				const glm::vec3 & eye, 
				const glm::vec3 & up);
	void SetEye(const glm::vec3 & eye);
	void SetPos(const glm::vec3 & pos);
	void SetViewport(const glm::vec4 & viewport);

	const glm::vec3 & GetUp() const;
	const glm::vec3 & GetEye() const;
	const glm::vec3 & GetPos() const;
	const glm::mat4 & GetView();
	const glm::mat4 & GetProj();
	const glm::vec4 & GetViewport() const;

	virtual void OnAdd() override {}
	virtual void OnDel() override {}
	virtual void OnUpdate(float dt) override {}

private:
    void Update();

private:
	Type _type;
	Info _info;
	bool _change;
	glm::mat4 _view;
	glm::mat4 _project;
	glm::vec4 _viewport;
	glm::vec3 _eye, _pos, _up;
};