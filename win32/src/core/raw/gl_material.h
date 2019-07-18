#pragma once

#include "gl_res.h"
#include "gl_mesh.h"
#include "gl_program.h"
#include "gl_texture2d.h"

class GLMaterial : public GLRes {
public:
    struct Item {
        enum TypeEnum {
            kTEX2D,
            kTEX3D,
        };

        std::string         mKey;
        const GLTexture2D * mTex2D;
        
        Item() 
            : mTex2D(nullptr) 
        { }

        Item(const std::string & key, const GLTexture2D * tex2D)
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
    void SetItem(const std::string & key, const T & val)
    {
        auto it = std::find(_items.begin(), _items.end(), key);
        if (it != _items.end()) { *it = Item(key, val); }
        else { _items.emplace_back(key, val); }
    }

    const std::vector<Item> & GetItems() const
    {
        return _items;
    }

private:
    const GLProgram *       _glProgram;
    const GLMesh    *       _glMesh;
    std::vector<Item>       _items;
};