#include "camera.h"
#include "../render/render.h"

Camera::Camera() : _change(true)
{ }

Camera::~Camera()
{ }

void Camera::InitPerspective(float fov, float w, float h, float n, float f)
{
	_change = true;
	_type = TypeEnum::kPERSPECTIVE;
	_info.mPersp.fov = glm::radians(fov);
	_info.mPersp.w = w; _info.mPersp.h = h;
	_info.mPersp.n = n; _info.mPersp.f = f;
}

void Camera::InitOrthogonal(float l, float r, float t, float b, float n, float f)
{
	_change = true;
	_type = TypeEnum::kORTHOGONAL;
	_info.mOrtho.l = l; _info.mOrtho.r = r;
	_info.mOrtho.t = t; _info.mOrtho.b = b;
	_info.mOrtho.n = n; _info.mOrtho.f = f;
}

void Camera::LookAt(const glm::vec3 & pos, const glm::vec3 & eye, const glm::vec3 & up)
{
	_up = up; _pos = pos; _eye = glm::normalize(eye - pos); _change = true;
}

void Camera::SetMask(const uint mask)
{
    _mask = mask;
}

void Camera::SetOrder(const uint order)
{
    _order = order;
}

void Camera::SetEye(const glm::vec3 & eye)
{
    _eye    = eye;
	_change = true;
}

void Camera::SetPos(const glm::vec3 & pos)
{
	_pos    = pos;
    _change = true;
}

void Camera::SetViewport(const glm::vec4 & viewport)
{
	_viewport = viewport;
}

uint Camera::GetMask() const
{
    return _mask;
}

uint Camera::GetOrder() const
{
    return _order;
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

const glm::mat4 & Camera::GetView() const
{
	Update();
	return _view;
}

const glm::mat4 & Camera::GetProj() const
{
	Update();
	return _project;
}

const glm::vec4 & Camera::GetViewport() const
{
	return _viewport;
}

void Camera::OnUpdate(float dt)
{
    CameraCommand command;
    command.mViewport   = GetViewport();
    command.mProj       = GetProj();
    command.mView       = GetView();
    command.mEye        = GetEye();
    command.mPos        = GetPos();
    command.mOrder      = GetOrder();
    command.mMask       = GetMask();
    if (_type = TypeEnum::kPERSPECTIVE)
    {
        command.mN = _info.mPersp.n;
        command.mF = _info.mPersp.f;
    }
    else
    {
        command.mN = _info.mOrtho.n;
        command.mF = _info.mOrtho.f;
    }
    Global::Ref().RefRender().Post(RenderCommand::TypeEnum::kCAMERA, command);
}

void Camera::Update() const
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
