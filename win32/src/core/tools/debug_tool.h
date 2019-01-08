#pragma once

#define CHECK_RET(cond, ...)	{ if (!(cond)) { return (__VA_ARGS__); } }