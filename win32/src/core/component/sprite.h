#pragma once

#include "component.h"

#include "../raw/gl_mesh.h"
#include "../raw/gl_program.h"
#include "../raw/gl_material.h"
#include "../raw/gl_texture2d.h"

class Sprite : public Component {
public:
    Sprite();
    virtual ~Sprite() {}
    virtual void OnAdd() override { }
    virtual void OnDel() override { }
	virtual void OnUpdate(float dt) override;

    void BindMaterial(const GLMaterial * material)
    {
        _material = material;
    }

    const GLMaterial * GetMaterial() const
    {
        return _material;
    }

protected:
    const GLMaterial * _material;
};