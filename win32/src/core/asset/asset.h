#pragma once

#include "../include.h"

class Res {
public:
	virtual ~Res()
	{ }

protected:
	Res()
	{ }

	Res(Res &&) = delete;
	Res(const Res &) = delete;
	Res & operator=(const Res &) = delete;
	Res & operator=(Res &&) = delete;
};