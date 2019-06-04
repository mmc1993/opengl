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
        char mName[RAW_NAME_LEN];
        const GLTexture2D * mTexture;
    };

public:
    GLMaterial(): _glMesh(nullptr), _glProgram(nullptr), _shininess(32.0f)
    {
        memset(_glTexture2Ds, 0, sizeof(_glTexture2Ds));
    }

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
        const char * name, const uint i)
    {
        _glTexture2Ds[i].mTexture = glTexture2D;

        std::copy(name, name + RAW_NAME_LEN, _glTexture2Ds[i].mName);
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
        if (i < MTLTEX2D_LEN)
        {
            auto & texture = _glTexture2Ds[i];
            if (texture.mTexture != nullptr)
            {
                return &texture;
            }
        }
        return nullptr;
    }

private:
    const GLMesh      * _glMesh;
    const GLProgram   * _glProgram;
    float               _shininess;
    Texture2D           _glTexture2Ds[MTLTEX2D_LEN];
};