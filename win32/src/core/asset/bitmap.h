#pragma once

#include "asset.h"

class Bitmap: public Asset {
public:
	Bitmap(int w, int h, int texfmt, int rawfmt, int pixtype, const std::string & url, const void * buffer): _GLID(0)
	{
		Init(w, h, texfmt, rawfmt, pixtype, url, buffer);
	}

    ~Bitmap()
    {
        glDeleteTextures(1, &_GLID);
    }

	void SetParameter(GLenum key, iint val)
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

    uint GetGLID() const
    {
		assert(_GLID != 0);
        return _GLID;
    }

private:
	void Init(int w, int h, int texfmt, int rawfmt, int pixtype, const std::string & url, const void * buffer)
	{
		_w = w; _h = h; _url = url;
		glGenTextures(1, &_GLID);
		glBindTexture(GL_TEXTURE_2D, _GLID);
		glTexImage2D(GL_TEXTURE_2D, 0, texfmt, w, h, 0, rawfmt, pixtype, buffer);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

private:
	int _w;
	int _h;
    uint _GLID;
	std::string _url;
};

using RenderTexture2D = Bitmap;
