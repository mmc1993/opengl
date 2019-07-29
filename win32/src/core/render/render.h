#pragma once

#include "../include.h"
#include "render_type.h"

class PipeState;

class Render {
public:
    Render(): _state(nullptr)
    {}

    ~Render()
    {
        SAFE_DELETE(_state);
    }

    MatrixStack &   GetMaterialStack();
    uint            GetVertexCount();
    uint            GetDrawCount();

    void PostCommand();

private:
    PipeState * _state;
};

