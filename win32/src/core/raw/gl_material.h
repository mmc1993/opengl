#pragma once

#include "gl_res.h"
#include "gl_mesh.h"
#include "gl_program.h"
#include "gl_texture2d.h"

class GLMaterial : public GLRes {
public:
    struct Item {
        enum TypeEnum {
            kNUMBER,
            kTEX2D,
            kTEX3D,
        };

        std::string mKey;
        std::any    mVal;
        TypeEnum    mType;
        
        template <class Val>
        Item(const TypeEnum type, const std::string & key, const Val & val)
            : mType(type), mKey(key), mVal(val)
        { }

        bool operator==(const std::string & key) const
        {
            return mKey == key;
        }

        Item() = default;
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

    template <class Val>
    void SetItem(const Item::TypeEnum type, const std::string & key, const Val & val)
    {
        auto it = std::find(_items.begin(), _items.end(), key);
        if (it == _items.end())
        {
            _items.emplace_back(type, key, val);
        }
        else
        {
            ASSERT_LOG(it->mVal.type() == typeid(Val), "{0}, {1}", 
                       it->mVal.type().name(), typeid(Val).name());
            ASSERT_LOG(it->mType == type, "{0}, {1}", it->mType, type);
            it->mVal.emplace<Val>(val);
        }
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