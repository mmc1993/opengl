#pragma once

#include "../include.h"

//  GL 资源基类

class GLRes : public NonCopy {
public:
	virtual ~GLRes()
    { }

protected:
    GLRes()
    { }
};