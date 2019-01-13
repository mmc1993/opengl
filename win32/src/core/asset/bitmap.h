#pragma once

#include "asset.h"
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "../third/stb_image.h"

class Bitmap: public Asset {
public:
    struct Data {
        int w;
        int h;
        int format;
        std::string url;
    };

public:
    Bitmap(Data && data, const void * buffer)
		: _data(std::move(data)), _GLID(0)
    {
		if (buffer != nullptr)
		{
			glGenTextures(1, &_GLID);
			glBindTexture(GL_TEXTURE_2D, _GLID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, data.format, _data.w, _data.h, 0, data.format, GL_UNSIGNED_BYTE, buffer);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

    ~Bitmap()
    {
        glDeleteTextures(1, &_GLID);
    }

    int GetW() const
    {
        return _data.w;
    }

    int GetH() const
    {
        return _data.h;
    }

    const std::string & GetURL() const
    {
        return _data.url;
    }

    GLuint GetGLID() const
    {
		assert(_GLID != 0);
        return _GLID;
    }

private:
    Data _data;
    GLuint _GLID;
};