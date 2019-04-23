#include "camera.h"

Camera::Camera() : _change(true)
{ }

Camera::~Camera()
{ }

void Camera::InitPerspective(float fov, float w, float h, float n, float f)
{
	_change = true;
	_type = Type::kPERSPECTIVE;
	_info.mPersp.fov = glm::radians(fov);
	_info.mPersp.w = w; _info.mPersp.h = h;
	_info.mPersp.n = n; _info.mPersp.f = f;
}

void Camera::InitOrthogonal(float l, float r, float t, float b, float n, float f)
{
	_change = true;
	_type = Type::kORTHOGONAL;
	_info.mOrtho.l = l; _info.mOrtho.r = r;
	_info.mOrtho.t = t; _info.mOrtho.b = b;
	_info.mOrtho.n = n; _info.mOrtho.f = f;
}

void Camera::LookAt(const glm::vec3 & pos, const glm::vec3 & eye, const glm::vec3 & up)
{
	_up = up; _pos = pos; _eye = glm::normalize(eye - pos); _change = true;
}

void Camera::SetEye(const glm::vec3 & eye)
{
	_eye = eye;
	auto right = glm::cross(_eye, glm::vec3(0, 1, 0));
	_up = glm::cross(right, _eye); _change = true;
}

void Camera::SetPos(const glm::vec3 & pos)
{
	_pos = pos; _change = true;
}

void Camera::SetViewport(const glm::vec4 & viewport)
{
	_viewport = viewport;
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

const glm::mat4 & Camera::GetView()
{
	Update();
	return _view;
}

const glm::mat4 & Camera::GetProj()
{
	Update();
	return _project;
}

const glm::vec4 & Camera::GetViewport() const
{
	return _viewport;
}

void Camera::Update()
{
	if (_change)
	{
		_change = false;

		switch (_type)
		{
		case Camera::kORTHOGONAL:
			_project = glm::ortho(_info.mOrtho.l, _info.mOrtho.r, 
								  _info.mOrtho.t, _info.mOrtho.b, 
								  _info.mOrtho.n, _info.mOrtho.f);
			break;
		case Camera::kPERSPECTIVE:
			_project = glm::perspective(_info.mPersp.fov, 
										_info.mPersp.w / _info.mPersp.h, 
										_info.mPersp.n, _info.mPersp.f);
			break;
		}
		_view = glm::lookAt(_pos, _pos + _eye, _up);
	}
}
