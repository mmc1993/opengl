#pragma once

#include "../include.h"
#include "../math/vec4.h"

class Camera {
public:
    Camera();
    ~Camera();

    void Init();
    void LookAt(const Vec4 & pos);
    void LookAt(float x, float y, float z);
};