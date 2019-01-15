#pragma once

#include "../include.h"
#include "../math/vec4.h"
#include "../math/matrix.h"
#include "../component/transform.h"
#include "object.h"

class Camera: public Object {
public:
    Camera();
    ~Camera();
    void Init(float fov, float width, float height, float near, float far);
    void LookAt(const glm::vec3 & pos, 
				const glm::vec3 & eye, 
				const glm::vec3 & up);

	void SetEye(const glm::vec3 & eye);
	void SetPos(const glm::vec3 & pos);
	void SetScale(float scale);
	void SetRotate(const glm::vec3 & rotate);

	float GetScale() const;
	const glm::vec3 & GetUp() const;
	const glm::vec3 & GetEye() const;
	const glm::vec3 & GetPos() const;
	const glm::vec3 & GetRotate() const;

	const glm::mat4 & GetProject();
	const glm::mat4 & GetView();

	glm::vec4 InPosition(Transform * transform);

private:
	void Update();

private:
	bool _change;
    glm::mat4 _project;
	glm::mat4 _view;
	glm::vec3 _eye, _pos, _up, _rotate;
	float _w, _h, _near, _far, _fov, _scale;
};