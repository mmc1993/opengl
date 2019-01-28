#pragma once

#include "asset.h"

class BitmapCube: public Asset {
public:
	BitmapCube(int w, int h, int fmt, const std::vector<std::string> & urls, const std::vector<const void *> & buffers): _GLID(0)
    {
		Init(w, h, fmt, urls, buffers);
	}
	
    ~BitmapCube()
    {
        glDeleteTextures(1, &_GLID);
    }

	void SetParameter(GLenum key, GLint val)
	{
		glBindTexture(GL_TEXTURE_2D, _GLID);
		glTexParameteri(GL_TEXTURE_2D, key, val);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

    const std::vector<std::string> & GetURLs() const
    {
        return _urls;
    }

    GLuint GetGLID() const
    {
		assert(_GLID != 0);
        return _GLID;
    }

private:
	void Init(int w, int h, int fmt, const std::vector<std::string> & urls, const std::vector<const void *> & buffers)
	{
		_urls = urls;
		glGenTextures(1, &_GLID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, _GLID);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		for (auto i = 0; i != buffers.size(); ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, buffers.at(i));
		}
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

private:
	GLuint _GLID;
	
	std::vector<std::string> _urls;
};