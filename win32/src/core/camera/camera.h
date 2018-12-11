#pragma once

#include "../include.h"
#include "../math/vec4.h"
#include "../math/matrix.h"

class Camera {
public:
    Camera();
    ~Camera();

    void Init(float fov, float wdivh, float near, float far);
    void LookAt(const Vec4 & pos);
    void LookAt(float x, float y, float z);
};