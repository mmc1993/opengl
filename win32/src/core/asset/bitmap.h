#pragma once

#include "asset.h"

class Bitmap: public Asset {
public:
    Bitmap(int w, int h, int fmt, const std::string & url, const void * buffer): _GLID(0)
    {
		Init(w, h, fmt, fmt, GL_UNSIGNED_BYTE, url, buffer);
	}
	
	Bitmap(int w, int h, int fmt1, int fmt2, int type, const std::string & url, const void * buffer): _GLID(0)
	{
		Init(w, h, fmt1, fmt2, type, url, buffer);
	}

    ~Bitmap()
    {
        glDeleteTextures(1, &_GLID);
    }

	void SetParameter(GLenum key, GLint val)
	{
		glBindTexture(GL_TEXTURE_2D, _GLID);
		glTexParameteri(GL_TEXTURE_2D, key, val);
		glBindTexture(GL_TEXTURE_2D, 0);
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
	void Init(int w, int h, int fmt1, int fmt2, int type, const std::string & url, const void * buffer)
	{
		_w = w; _h = h; _url = url;
		glGenTextures(1, &_GLID);
		glBindTexture(GL_TEXTURE_2D, _GLID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, fmt1, w, h, 0, fmt2, type, buffer);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

private:
	int _w;
	int _h;
	std::string _url;
    GLuint _GLID;
};