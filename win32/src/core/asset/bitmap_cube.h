#pragma once

#include "asset.h"

class BitmapCube: public Asset {
public:
	BitmapCube(int w, int h, 
			   int texfmt, int glfmt, int gltype, 
			   const std::vector<std::string> & urls, 
			   const std::vector<const void *> & buffers) : _GLID(0)
	{
		Init(w, h, texfmt, glfmt, gltype, urls, buffers);
	}
	
    ~BitmapCube()
    {
        glDeleteTextures(1, &_GLID);
    }

	void SetParameter(GLenum key, GLint val)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, _GLID);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, key, val);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
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
	void Init(int w, int h, 
			  int texfmt, int glfmt, int gltype, 
			  const std::vector<std::string> & urls, 
			  const std::vector<const void *> & buffers)
	{
		_urls = urls;
		glGenTextures(1, &_GLID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, _GLID);
		for (auto i = 0u; i != buffers.size(); ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, texfmt, w, h, 0, glfmt, gltype, i < buffers.size() ? buffers.at(i) : nullptr);
		}
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

private:
	GLuint _GLID;
	
	std::vector<std::string> _urls;
};