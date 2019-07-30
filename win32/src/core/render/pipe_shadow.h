#pragma once

#include "pipe.h"

class PipeShadow : public Pipe {
public:
    virtual ~PipeShadow() {}
    virtual void OnAdd(Renderer * renderer, PipeState * state);
    virtual void OnDel(Renderer * renderer, PipeState * state);
    virtual void OnUpdate(Renderer * renderer, PipeState * state);

private:
    using BakeFunc_t = void (PipeShadow::*)(Renderer * renderer, PipeState * state, uint texture, const LightCommand * light);

    void DoBakeShadow(Renderer * renderer, PipeState * state, uint texture, const LightCommand * light, const BakeFunc_t bakefunc);
    void OnBakeShadow2D(Renderer * renderer, PipeState * state, uint texture, const LightCommand * light);
    void OnBakeShadow3D(Renderer * renderer, PipeState * state, uint texture, const LightCommand * light);
    void PostBatch(Renderer * renderer, PipeState * state, const LightCommand * light);
};