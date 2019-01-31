#pragma once

#include "../include.h"

#define PARAM_1				std::placeholders::_1
#define PARAM_2				std::placeholders::_2
#define PARAM_3				std::placeholders::_3
#define PARAM_4				std::placeholders::_4
#define PARAM_5				std::placeholders::_5
#define BIND(func, ...)		std::bind(&func, __VA_ARGS__)

