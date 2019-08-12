#pragma once

#include "gl_res.h"
#include "gl_mesh.h"
#include "gl_program.h"
#include "gl_texture2d.h"

class GLMaterial : public GLRes {
public:
    struct Texture {
        enum TypeEnum {
            kTEX2D,
            kTEX3D,
        };

        std::string         mKey;
        const GLTexture2D * mTex2D;
        
        Texture() : mTex2D(nullptr) 
        { }

        Texture(const std::string & key, const GLTexture2D * tex2D)
            : mKey(key), mTex2D(tex2D)
        { }

        bool operator==(const std::string & key) const
        {
            return mKey == key;
        }
    };

public:
    GLMaterial() : _glMesh(nullptr), _glProgram(nullptr)
    { }

    void SetMesh(const GLMesh * glMesh)
    {
        _glMesh = glMesh;
    }

    const GLMesh * GetMesh() const
    {
        return _glMesh;
    }

    void SetProgram(const GLProgram * glProgram)
    {
        _glProgram = glProgram;
    }

    const GLProgram * GetProgram() const
    {
        return _glProgram;
    }

    template <class T>
    void SetTexture(const std::string & key, const T & val)
    {
        auto it = std::find(_textures.begin(), _textures.end(), key);
        if (it != _textures.end()) { *it = Texture(key, val); }
        else { _textures.emplace_back(key, val); }
    }

    const std::vector<Texture> & GetTextures() const
    {
        return _textures;
    }

private:
    const GLMesh    *       _glMesh;
    const GLProgram *       _glProgram;
    std::vector<Texture>    _textures;
};