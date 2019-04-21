#pragma once

#include "../include.h"

namespace time_tool {
	inline float Now(float offset = 0.0f)
	{
		using std::chrono::duration_cast;
		using typename std::chrono::milliseconds;
		using typename std::chrono::high_resolution_clock;
		return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() * 0.001f + offset;
	}

    inline float UnLerp(float step, float beg, float end)
    {
        assert(step != 0);
        return (end - beg) / step;
    }
}