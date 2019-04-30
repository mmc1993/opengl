#pragma once

#include "../include.h"

namespace time_tool {
	inline float Now(float offset = 0.0f)
	{
        return static_cast<float>(glfwGetTime()) + offset;
	}
}