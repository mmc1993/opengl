#include "transform.h"
#include "../object/object.h"

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
    _translate.x() = x;
    _translate.y() = y;
    _translate.z() = z;
}

void Transform::Translate(const Eigen::Vector3f & vec)
{
    Translate(vec.x(), vec.y(), vec.z());
}

void Transform::Translate(const Eigen::Vector4f & vec)
{
    Translate(vec.x(), vec.y(), vec.z());
}

void Transform::Rotate(const Eigen::Quaternionf & rotate)
{
    _isChange = true;
    _rotate = rotate;
}

void Transform::Rotate(float x, float y, float z, float a)
{
    Rotate(Eigen::Quaternionf(
        std::cos(a * 0.5f),
        std::sin(a * 0.5f) * x,
        std::sin(a * 0.5f) * y,
        std::sin(a * 0.5f) * z));
}

void Transform::Rotate(const Eigen::Vector3f & vec4, float a)
{
    Rotate(vec4.x(), vec4.y(), vec4.z(), a);
}

void Transform::Scale(float x, float y, float z)
{
    _isChange = true;
    _scale.x() = x;
    _scale.y() = y;
    _scale.z() = z;
}

void Transform::Scale(const Eigen::Vector3f & vec)
{
    Scale(vec.x(), vec.y(), vec.z());
}

void Transform::Scale(const Eigen::Vector4f & vec)
{
    Scale(vec.x(), vec.y(), vec.z());
}

const Eigen::Quaternionf & Transform::GetRotateQuat() const
{
    return _rotate;
}

const Eigen::Vector3f & Transform::GetPosition() const
{
    return _translate;
}

const Eigen::Vector3f & Transform::GetScale() const
{
    return _scale;
}

Eigen::Vector3f Transform::GetRotate() const
{
    auto a = std::acos(_rotate.w()) * 2.0f;
    return Eigen::Vector3f(
        _rotate.x() / (std::asin(a * 0.5f) * 2.0f),
        _rotate.y() / (std::asin(a * 0.5f) * 2.0f),
        _rotate.z() / (std::asin(a * 0.5f) * 2.0f));
}

Matrix Transform::GetMatrixFrom(const Object * target)
{
    auto matrix = Matrix::Identity();
    for (auto owner = GetOwner()->GetParent(); 
        owner != target && owner != nullptr; 
        owner = owner->GetParent())
    {
        auto transform = owner->GetComponent<Transform>();
        if (transform != nullptr)
        {
            matrix = transform->GetMatrix() * matrix;
        }
    }
    return matrix * _matrix;
}

void Transform::UpdateMatrix()
{
    if (_isChange)
    {
        _isChange = false;
        _matrix = Eigen::Translation3f(_translate)
            * _rotate.toRotationMatrix()
            * Eigen::Scaling(_scale);
        _matrix = GetMatrixFrom();
    }
}
