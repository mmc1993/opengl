#pragma once

#include "../include.h"

#define CHECK_RET(cond, ...)	{ if (!(cond)) { return __VA_ARGS__; } }

#define ASSERT_RET(cond, ...)	{ if (!(cond)) { assert(false); return __VA_ARGS__; } }