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

    void Rotate(const Eigen::Quaternionf & rotate);
    void Rotate(float x, float y, float z, float a);
    void Rotate(const Eigen::Vector3f & vec, float a);

    void Scale(float x, float y, float z);
    void Scale(const Eigen::Vector3f & vec);
    void Scale(const Eigen::Vector4f & vec);

    Transform & AddTranslate(float x, float y, float z);
    Transform & AddTranslate(const Eigen::Vector3f & vec);
    Transform & AddTranslate(const Eigen::Vector4f & vec);

    Transform & AddRotate(const Eigen::Quaternionf & rotate);
    Transform & AddRotate(float x, float y, float z, float a);
    Transform & AddRotate(const Eigen::Vector3f & vec, float a);

    Transform & AddScale(float x, float y, float z);
    Transform & AddScale(const Eigen::Vector3f & vec);
    Transform & AddScale(const Eigen::Vector4f & vec);

    const Eigen::Quaternionf & GetRotateQuat() const;
    const Eigen::Vector3f & GetPosition() const;
    const Eigen::Vector3f & GetScale() const;
    Eigen::Vector3f GetRotate() const;

    const Matrix & GetMatrix() const { return _matrix; }

    Matrix GetMatrixFrom(const Object * target = nullptr);

private:
    void UpdateMatrix();

private:
    Eigen::Quaternionf _rotate;
    Eigen::Vector3f _translate;
    Eigen::Vector3f _scale;
    Matrix _matrix;
    bool _isChange;
};