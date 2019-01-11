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
    const glm::vec3 & GetUp() const;
    const glm::vec3 & GetEye() const;
    const glm::vec3 & GetPos() const;
	void Bind();
	void Free();

private:
    glm::mat4 _project;
	glm::mat4 _modelview;
	glm::vec3 _eye, _pos, _up;
};