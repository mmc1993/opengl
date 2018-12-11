#pragma once

#include "math.h"

namespace math {
    inline glm::vec3 Lerp(const glm::vec3 & v1, const glm::vec3 & v2, float p)
    {
        return (v2 - v1) * p + v1;
    }

    inline float Unlerp(const glm::vec3 & v1, const glm::vec3 & v2, const glm::vec3 & v3)
    {
        return glm::length(v3 - v1) / glm::length(v2 - v1);
    }
}