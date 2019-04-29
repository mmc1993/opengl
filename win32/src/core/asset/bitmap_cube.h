#pragma once

#include "asset.h"

class BitmapCube: public Res {
public:
	BitmapCube(int w, int h, 
			   int texfmt, int rawfmt, int pixtype, 
			   const std::vector<std::string> & urls, 
			   const std::vector<const void *> & buffers) : _GLID(0)
	{
		Init(w, h, texfmt, rawfmt, pixtype, &urls, &buffers);
	}

    BitmapCube(int w, int h, int texfmt, int rawfmt, int pixtype) : _GLID(0)
    {
        Init(w, h, texfmt, rawfmt, pixtype, nullptr, nullptr);
    }
	
    ~BitmapCube()
    {
        glDeleteTextures(1, &_GLID);
    }

	void SetParameter(GLenum key, iint val)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, _GLID);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, key, val);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

    const std::vector<std::string> & GetURLs() const
    {
        return _urls;
    }

    uint GetGLID() const
    {
		assert(_GLID != 0);
        return _GLID;
    }

private:
	void Init(int w, int h, 
			  int texfmt, int rawfmt, int pixtype, 
			  const std::vector<std::string> * urls, 
			  const std::vector<const void *> * buffers)
	{
		glGenTextures(1, &_GLID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, _GLID);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, texfmt, w, h, 0, rawfmt, pixtype, buffers != nullptr ? buffers->at(0) : nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, texfmt, w, h, 0, rawfmt, pixtype, buffers != nullptr ? buffers->at(1) : nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, texfmt, w, h, 0, rawfmt, pixtype, buffers != nullptr ? buffers->at(2) : nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, texfmt, w, h, 0, rawfmt, pixtype, buffers != nullptr ? buffers->at(3) : nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, texfmt, w, h, 0, rawfmt, pixtype, buffers != nullptr ? buffers->at(4) : nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, texfmt, w, h, 0, rawfmt, pixtype, buffers != nullptr ? buffers->at(5) : nullptr);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        if (urls != nullptr){ _urls = *urls; }
	}

private:
	uint _GLID;
	
	std::vector<std::string> _urls;
};

using RenderTexture3D = BitmapCube;
