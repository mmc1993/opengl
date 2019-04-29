#pragma once

#include "../include.h"

class Res : public NoCopy {
public:
	virtual ~Res() 
    {
    }

protected:
	Res() 
    {
    }

	Res(const Res &) = delete;
	Res & operator=(const Res &) = delete;
};