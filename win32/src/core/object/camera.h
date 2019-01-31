#pragma once

#include "object.h"

class Camera: public Object {
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
	void InitOrthogonal(float l, float r, float t, float b, float n, float f);
	void InitPerspective(float fov, float w, float h, float n, float f);

    //void Init(float fov, float width, float height, float near, float far);
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
	const glm::mat4 & GetProject();
	const glm::vec4 & GetViewport() const;

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
	float _w, _h, _near, _far, _fov;
};