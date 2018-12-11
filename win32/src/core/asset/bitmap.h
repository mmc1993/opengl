#pragma once

#include "../include.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../third/stb_image.h"

class Bitmap {
public:
    struct ImageInfo {
        int w;
        int h;
        int channel;
        std::string url;
    };

public:
    Bitmap(): _GLID(0)
    {
    }

    ~Bitmap()
    {
        assert(_GLID == 0);
    }

    bool Load(const std::string & url)
    {
        stbi_set_flip_vertically_on_load(1);

        auto data = stbi_load(url.c_str(), &_imageInfo.w, &_imageInfo.h, &_imageInfo.channel, 0);
        if (data != nullptr)
        {
            _imageInfo.url = url;
            glGenTextures(1, &_GLID);
            glBindTexture(GL_TEXTURE_2D, _GLID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _imageInfo.w, _imageInfo.h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glBindTexture(GL_TEXTURE_2D, 0);
            stbi_image_free(data);
            return true;
        }
        return false;
    }

    void Free()
    {
        glDeleteTextures(1, &_GLID);
    }

    int GetW() const
    {
        return _imageInfo.w;
    }

    int GetH() const
    {
        return _imageInfo.h;
    }

    const std::string & GetURL() const
    {
        return _imageInfo.url;
    }

    GLuint GetGLID() const
    {
        return _GLID;
    }

private:
    ImageInfo _imageInfo;

    GLuint _GLID;
};