#include "transform.h"
#include "../object/object.h"

Transform::Transform()
	: _isChange(true)
	, _scale(1, 1, 1)
	, _translate(0, 0, 0)
	, _rotate(glm::vec3(0, 0, 0))
{
}

Transform::~Transform()
{
}

void Transform::OnAdd()
{
}

void Transform::OnDel()
{
}

void Transform::OnUpdate(float dt)
{
    UpdateMatrix();
}

void Transform::Translate(float x, float y, float z)
{
    _isChange = true;
    _translate.x = x;
    _translate.y = y;
    _translate.z = z;
}

void Transform::Translate(const glm::vec3 & vec)
{
    Translate(vec.x, vec.y, vec.z);
}

void Transform::Translate(const glm::vec4 & vec)
{
    Translate(vec.x, vec.y, vec.z);
}

void Transform::Rotate(const glm::quat & rotate)
{
    _isChange = true;
    _rotate = rotate;
}

void Transform::Rotate(float x, float y, float z, float a)
{
    Rotate(glm::vec3(x, y, z), a);
}

void Transform::Rotate(const glm::vec3 & vec, float a)
{
    Rotate(glm::quat(glm::angleAxis(a, vec)));
}

void Transform::Scale(float x)
{
	Scale(x, x, x);
}

void Transform::Scale(float x, float y, float z)
{
    _isChange = true;
    _scale.x = x;
    _scale.y = y;
    _scale.z = z;
}

void Transform::Scale(const glm::vec3 & vec)
{
    Scale(vec.x, vec.y, vec.z);
}

void Transform::Scale(const glm::vec4 & vec)
{
    Scale(vec.x, vec.y, vec.z);
}

Transform & Transform::AddTranslate(float x, float y, float z)
{
    Translate(_translate.x + x, _translate.y + y, _translate.z + z);
    return *this;
}

Transform & Transform::AddTranslate(const glm::vec3 & vec)
{
    return AddTranslate(vec.x, vec.y, vec.z);
}

Transform & Transform::AddTranslate(const glm::vec4 & vec)
{
    return AddTranslate(vec.x, vec.y, vec.z);
}

Transform & Transform::AddRotate(const glm::quat & rotate)
{
    Rotate(_rotate * rotate);
    return *this;
}

Transform & Transform::AddRotate(float x, float y, float z, float a)
{
    return AddRotate(glm::vec3(x, y, z), a);
}

Transform & Transform::AddRotate(const glm::vec3 & vec, float a)
{
    return AddRotate(glm::quat(glm::angleAxis(a, vec)));
}

Transform & Transform::AddScale(float x, float y, float z)
{
    Scale(_scale.x + x, _scale.y + y, _scale.z + z);
    return *this;
}

Transform & Transform::AddScale(const glm::vec3 & vec)
{
    return AddScale(vec.x, vec.y, vec.z);
}

Transform & Transform::AddScale(const glm::vec4 & vec)
{
    return AddScale(vec.x, vec.y, vec.z);
}

const glm::quat & Transform::GetRotateQuat() const
{
    return _rotate;
}

const glm::vec3 & Transform::GetPosition() const
{
    return _translate;
}

const glm::vec3 & Transform::GetScale() const
{
    return _scale;
}

glm::vec3 Transform::GetRotate() const
{
    return glm::eulerAngles(_rotate);
}

const glm::mat4 & Transform::GetMatrix()
{ 
    UpdateMatrix();
    return _matrix; 
}

glm::mat4 Transform::GetMatrixFromRoot()
{
	auto matrix = GetMatrix();
	auto parent = GetOwner()->GetParent();
	while (parent != nullptr)
	{
		matrix = parent->GetTransform()->GetMatrix() * matrix;
		parent = parent->GetParent();
	}
	return matrix;
}

glm::mat4 Transform::GetRotateFromRoot()
{
	auto matrix = GetRotateQuat();
	auto parent = GetOwner()->GetParent();
	while (parent != nullptr)
	{
		matrix = parent->GetTransform()->GetRotateQuat() * matrix;
		parent = parent->GetParent();
	}
	return (glm::mat4)matrix;
}

glm::vec3 Transform::GetWorldPosition()
{
	return glm::vec3(GetMatrixFromRoot() * glm::vec4(0, 0, 0, 1));
}

glm::vec3 Transform::ApplyRotate(const glm::vec3 & vec3)
{
	return glm::normalize(glm::mat3(glm::transpose(glm::inverse(GetMatrix()))) * vec3);
}

void Transform::UpdateMatrix()
{
	if (_isChange)
	{
		_isChange = false;
		auto t = glm::translate(glm::mat4(1), _translate);
		auto s = glm::scale(glm::mat4(1), _scale);
		_matrix = t * (glm::mat4)_rotate * s;
	}
}
