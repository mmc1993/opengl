#pragma once

#include "math.h"

using Vec4 = Eigen::Vector4f;

namespace math {
    inline Vec4 Lerp(const Vec4 & v1, const Vec4 & v2, float p)
    {
        return (v2 - v1) * p + v1;
    }

    inline float Unlerp(const Vec4 & v1, const Vec4 & v2, const Vec4 & v3)
    {
        return (v3 - v1).norm() / (v2 - v1).norm();
    }
}