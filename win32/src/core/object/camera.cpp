#include "camera.h"
#include "../mmc.h"
#include "../render/render.h"

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::Init(float fov, float width, float height, float near, float far)
{
    _project = glm::perspective(glm::radians(fov), width / height, near, far);
}

void Camera::LookAt(const glm::vec3 & pos, const glm::vec3 & eye, const glm::vec3 & up)
{
    _modelview = glm::lookAt(pos, eye, up);
}

const glm::vec3 & Camera::GetUp() const
{
    return _up;
}

const glm::vec3 & Camera::GetEye() const
{
    return _eye;
}

const glm::vec3 & Camera::GetPos() const
{
    return _pos;
}

const glm::mat4 & Camera::GetProject() const
{
	return _project;
}

const glm::mat4 & Camera::GetModelView() const
{
	return _modelview;
}
