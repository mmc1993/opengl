#include "camera.h"
#include "../mmc.h"
#include "../render/render.h"

Camera::Camera()
	: _change(true)
	, _scale(1)
	, _rotate(glm::vec3(0, glm::radians(-90.0f), 0))
{
}

Camera::~Camera()
{
}

void Camera::Init(float fov, float width, float height, float near, float far)
{
	_change = true;
	_w = width; _h = height;
	_fov = glm::radians(fov);
	_near = near; _far = far;
}

void Camera::LookAt(const glm::vec3 & pos, const glm::vec3 & eye, const glm::vec3 & up)
{
	_up = up; _pos = pos; _eye = eye; _change = true;
}

void Camera::SetEye(const glm::vec3 & eye)
{
	_eye = eye;
	auto right = glm::cross(_eye, glm::vec3(0, 1, 0));
	_up = glm::cross(right, _eye);
	_change = true;
}

void Camera::SetPos(const glm::vec3 & pos)
{
	_pos = pos; _change = true;
}

void Camera::SetScale(float scale)
{
	_scale = scale; _change = true;
}

void Camera::SetRotate(const glm::vec3 & rotate)
{
	_rotate = rotate; _change = true;
}

float Camera::GetScale() const
{
	return _scale;
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

const glm::mat4 & Camera::GetProject()
{
	Update();
	return _project;
}

const glm::mat4 & Camera::GetView()
{
	Update();
	return _view;
}

void Camera::Update()
{
	if (_change)
	{
		_project = glm::perspective(_fov * _scale, _w / _h, _near, _far);
		_view = glm::lookAt(_pos, _pos + _eye, _up);
		_change = false;
	}
}
