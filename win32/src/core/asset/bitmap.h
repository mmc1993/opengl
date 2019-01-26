#pragma once

#include "asset.h"
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "../third/stb_image.h"

class Bitmap: public Asset {
public:
    Bitmap(int w, int h, int fmt, const std::string & url, const void * buffer): _GLID(0)
    {
		Init(w, h, fmt, fmt, GL_UNSIGNED_BYTE, url, buffer);
	}
	
	Bitmap(int w, int h, int imgfmt, int glfmt, int type, const std::string & url, const void * buffer): _GLID(0)
	{
		Init(w, h, imgfmt, glfmt, type, url, buffer);
	}

    ~Bitmap()
    {
        glDeleteTextures(1, &_GLID);
    }

	void SetParameter(GLenum key, GLint val)
	{
		glTexParameteri(GL_TEXTURE_2D, key, val);
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
	void Init(int w, int h, int imgfmt, int glfmt, int type, const std::string & url, const void * buffer)
	{
		_w = w; _h = h; _url = url;
		glGenTextures(1, &_GLID);
		glBindTexture(GL_TEXTURE_2D, _GLID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, imgfmt, w, h, 0, glfmt, type, buffer);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

private:
	int _w;
	int _h;
	std::string _url;
    GLuint _GLID;
};