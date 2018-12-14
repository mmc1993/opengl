#include "camera.h"

Camera::Camera()
    : _isChange(true)
{
}

Camera::~Camera()
{
}

void Camera::Init(float fov, float wdivh, float near, float far)
{
    _project = glm::perspective(fov, wdivh, near, far);
}

void Camera::LookAt(const glm::vec3 & eye, const glm::vec3 & pos, const glm::vec3 & up)
{
    _view = glm::lookAt(eye, pos, up);
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

void Camera::Apply()
{
	//	TODO MMC
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMultMatrixf(&_project[0][0]);
}

