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
        _rotate.x() / std::asin(_rotate.x()) * 2.0f,
        _rotate.y() / std::asin(_rotate.y()) * 2.0f,
        _rotate.z() / std::asin(_rotate.z()) * 2.0f);
}

Matrix Transform::GetMatrixFrom(const Object * target)
{
    UpdateMatrix();
    return Matrix();
}

void Transform::UpdateMatrix()
{
    //  ²¹¹¦¿Î
    if (_isChange)
    {
        _isChange = false;
        _matrix.setIdentity();
        auto owner = GetOwner();
        while (owner != nullptr)
        {
             
        }
        if (owner == nullptr) { return; }



        Eigen::Translation3f(_translate);
        Eigen::Scaling(_scale);
        //Eigen::AngleAxisd()
        //Eigen::Isometry3d
        //Matrix::
    }
}
