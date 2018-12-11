#pragma once

#include "math.h"

namespace math {
    inline Eigen::Vector3f Lerp(const Eigen::Vector3f & v1, const Eigen::Vector3f & v2, float p)
    {
        return (v2 - v1) * p + v1;
    }

    inline float Unlerp(const Eigen::Vector3f & v1, const Eigen::Vector3f & v2, const Eigen::Vector3f & v3)
    {
        return (v3 - v1).norm() / (v2 - v1).norm();
    }
}