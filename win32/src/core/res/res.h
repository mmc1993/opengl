#pragma once

#include "../include.h"

class Res : public NonCopy {
public:
	virtual ~Res() 
    { }

protected:
	Res() 
    { }

	Res(const Res &) = delete;
	Res & operator=(const Res &) = delete;
};