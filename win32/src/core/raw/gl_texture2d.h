#pragma once

#include "gl_res.h"

class GLTexture2D : public GLRes {
public:
    GLTexture2D(): _w(0), _h(0), _id(0)
    { }

    ~GLTexture2D()
    {
        glDeleteTextures(1, &_id);
    }

    void Init(uint rawFormat, uint texFormat, uint type, uint w, uint h, const uchar * data)
    {
        ASSERT_LOG(_id == 0, "id != 0");
        glGenTextures(1, &_id);
        ASSERT_LOG(_id != 0, "Init Error");
        glBindTexture(  GL_TEXTURE_2D, _id);
        glTexImage2D(   GL_TEXTURE_2D, 0, rawFormat, w, h, 0, texFormat, type, data);
        glBindTexture(  GL_TEXTURE_2D, 0);
        _w = w; _h = h;
    }

    void SetParam(uint key, iint val) const
    {
        glBindTexture(  GL_TEXTURE_2D, _id);
        glTexParameteri(GL_TEXTURE_2D, key, val);
        glBindTexture(  GL_TEXTURE_2D, 0);
    }

    uint GetID() const { return _id; }
    uint GetW() const { return _w; }
    uint GetH() const { return _h; }

private:
    uint _w;
    uint _h;
    uint _id;
};