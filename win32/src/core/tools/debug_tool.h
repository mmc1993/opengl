#pragma once

#include "../include.h"

#define CHECK_RET(cond, ...)	        { if (!(cond)) { return __VA_ARGS__; } }

#define ASSERT_RET(cond, ...)	        { if (!(cond)) { assert(false); return __VA_ARGS__; } }

#define ASSERT_LOG(cond, fmt, ...)      { if (!(cond)) { std::cout << "Line: " << __LINE__ << " " << "Func: " << __FUNCTION__ << " " << SFormat(fmt, __VA_ARGS__); abort(); } }
