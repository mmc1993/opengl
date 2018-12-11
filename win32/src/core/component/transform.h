#pragma once

#include "component.h"
#include "../math/vec4.h"
#include "../math/matrix.h"

class Transform : public Component {
public:
    virtual void OnAdd() override;
    virtual void OnDel() override;
    virtual void OnUpdate(float dt) override;

    void Translate(float x, float y, float z);
    void Translate(const Eigen::Vector3f & vec);
    void Translate(const Eigen::Vector4f & vec);

    void Rotate(const Eigen::Quaternionf & quat);
    void Rotate(float x, float y, float z, float a);
    void Rotate(const Eigen::Vector3f & vec, float a);

    void Scale(float x, float y, float z);
    void Scale(const Eigen::Vector3f & vec);
    void Scale(const Eigen::Vector4f & vec);

    const Eigen::Quaternionf & GetRotateQuat() const;
    const Eigen::Vector3f & GetPosition() const;
    const Eigen::Vector3f & GetScale() const;
    Eigen::Vector3f GetRotate() const;

    const Matrix & GetMatrix() const { return _matrix; }

    Matrix GetMatrixFrom(const Object * target);

private:
    void UpdateMatrix();

private:
    Eigen::Quaternionf _rotate;
    Eigen::Vector3f _translate;
    Eigen::Vector3f _scale;
    Matrix _matrix;
    bool _isChange;
};