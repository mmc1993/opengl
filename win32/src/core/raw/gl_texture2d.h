#pragma once

#include "gl_res.h"

class GLTexture2D : public GLRes {
public:
    uint GetID() const
    {
        return _id;
    }

private:
    uint _id;
};