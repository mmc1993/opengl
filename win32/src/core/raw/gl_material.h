#pragma once

#include "gl_res.h"
#include "gl_mesh.h"
#include "gl_program.h"
#include "gl_texture2d.h"

class GLMaterial : public GLRes {
public:
    struct Texture2D
    {
        //  材质引用的每一个纹理都必须对应一个名字, 
        //  这个名字会对应到Shader中的uniform变量.
        char mName[MTLTEX_NAME_BYTELEN];
        const GLTexture2D * mTexture;
    };

public:
    GLMaterial(): _glMesh(nullptr), _glProgram(nullptr)
    {
        memset(_glTexture2Ds, 0, sizeof(_glTexture2Ds));
    }

    void SetMesh(GLMesh * glMesh)
    {
        _glMesh = glMesh;
    }

    void SetProgram(GLProgram * glProgram)
    {
        _glProgram = glProgram;
    }

    void SetTexture2D(const GLTexture2D * glTexture2D, const std::string & name, uint i)
    {
        _glTexture2Ds[i].mTexture = glTexture2D;

        memcpy(_glTexture2Ds[i].mName, name.c_str(), name.size());
    }

    const GLMesh * GetMesh() const
    {
        return _glMesh;
    }

    const GLProgram * GetProgram() const
    {
        return _glProgram;
    }

    const Texture2D * GetTexture2Ds(uint i) const
    {
        if (i < MTLTEX2D_LEN)
        {
            return &_glTexture2Ds[i];
        }
        return nullptr;
    }

private:
    const GLMesh      * _glMesh;
    const GLProgram   * _glProgram;
    Texture2D           _glTexture2Ds[MTLTEX2D_LEN];
};