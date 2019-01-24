#pragma once

#include "asset.h"
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "../third/stb_image.h"

class Bitmap: public Asset {
public:
    Bitmap(int w, int h, int fmt, const std::string & url, const void * buffer)
		: _w(w), _h(h), _fmt(fmt), _url(url)
    {
		if (buffer != nullptr)
		{
			glGenTextures(1, &_GLID);
			glBindTexture(GL_TEXTURE_2D, _GLID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, buffer);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

    ~Bitmap()
    {
        glDeleteTextures(1, &_GLID);
    }

    int GetW() const
    {
        return _w;
    }

    int GetH() const
    {
        return _h;
    }

    const std::string & GetURL() const
    {
        return _url;
    }

    GLuint GetGLID() const
    {
		assert(_GLID != 0);
        return _GLID;
    }

private:
	int _w;
	int _h;
	int _fmt;
	std::string _url;
    GLuint _GLID;
};