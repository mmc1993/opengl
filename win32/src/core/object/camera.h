#pragma once

#include "../include.h"
#include "../math/vec4.h"
#include "../math/matrix.h"
#include "object.h"

class Camera: public Object {
public:
    Camera();
    ~Camera();

    void Init(float fov, float wdivh, float near, float far);
    void LookAt(const Eigen::Vector3f & pos);
    void LookAt(float x, float y, float z);
};