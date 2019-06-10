#pragma once

#include "gl_res.h"
#include "gl_mesh.h"
#include "gl_program.h"
#include "gl_texture2d.h"

class GLMaterial : public GLRes {
public:
    struct Texture2D
    {
        std::string mDesc; /*±êÊ¶·û*/
        const GLTexture2D * mTex;
    };

public:
    GLMaterial()
        : _glMesh(nullptr)
        , _glProgram(nullptr)
        , _shininess(0)
    { }

    void SetMesh(const GLMesh * glMesh)
    {
        _glMesh = glMesh;
    }

    void SetProgram(const GLProgram * glProgram)
    {
        _glProgram = glProgram;
    }

    void SetShininess(const float shininess)
    {
        _shininess = shininess;
    }

    void SetTexture2D(
        const GLTexture2D * glTexture2D, 
        const std::string & desc, 
        const uint i)
    {
        if (i < _glTexture2Ds.size())
        {
            _glTexture2Ds.at(i).mDesc = desc;
            _glTexture2Ds.at(i).mTex = glTexture2D;
        }
        else
        {
            Texture2D texture;
            texture.mDesc   = desc;
            texture.mTex    = glTexture2D;
            _glTexture2Ds.push_back(texture);
        }
    }

    const GLMesh * GetMesh() const
    {
        return _glMesh;
    }

    float GetShininess() const
    {
        return _shininess;
    }

    const GLProgram * GetProgram() const
    {
        return _glProgram;
    }

    const Texture2D * GetTexture2Ds(uint i) const
    {
        return i < _glTexture2Ds.size()
            ? &_glTexture2Ds.at(i)
            : nullptr;
    }

private:
    float                   _shininess;
    const GLMesh *          _glMesh;
    const GLProgram *       _glProgram;
    std::vector<Texture2D>  _glTexture2Ds;
};