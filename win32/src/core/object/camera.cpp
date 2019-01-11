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

void Camera::Bind()
{
	mmc::mRender.GetMatrix().Identity(Render::Matrix::kPROJECT);
	mmc::mRender.GetMatrix().Mul(Render::Matrix::kPROJECT, _project);
	mmc::mRender.GetMatrix().Identity(Render::Matrix::kMODELVIEW);
	mmc::mRender.GetMatrix().Mul(Render::Matrix::kMODELVIEW, _modelview);
}

void Camera::Free()
{
	mmc::mRender.GetMatrix().Pop(Render::Matrix::kPROJECT);
	mmc::mRender.GetMatrix().Pop(Render::Matrix::kMODELVIEW);
}
