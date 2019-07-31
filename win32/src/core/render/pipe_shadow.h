#pragma once

#include "pipe.h"

//  “ı”∞π‹µ¿
class PipeShadow : public Pipe {
public:
    virtual void OnAdd(Renderer * renderer, PipeState * state) override;
    virtual void OnDel(Renderer * renderer, PipeState * state) override;
    virtual void OnUpdate(Renderer * renderer, PipeState * state) override;

private:
    using BakeFunc_t = void (PipeShadow::*)(Renderer * renderer, PipeState * state, uint texture, const LightCommand * light);

    void BakeShadow(Renderer * renderer, PipeState * state, uint texture, const LightCommand * light, const BakeFunc_t bakefunc);
    void OnBakeShadow2D(Renderer * renderer, PipeState * state, uint texture, const LightCommand * light);
    void OnBakeShadow3D(Renderer * renderer, PipeState * state, uint texture, const LightCommand * light);
    void PostBatch(Renderer * renderer, PipeState * state, const LightCommand * light);
};