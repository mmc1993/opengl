#pragma once

#include "pipe.h"

//  —”≥Ÿ‰÷»æπ‹µ¿
class PipeDeferred : public Pipe {
public:
    virtual void OnAdd(Renderer * renderer, PipeState * state) override;
    virtual void OnDel(Renderer * renderer, PipeState * state) override;
    virtual void OnUpdate(Renderer * renderer, PipeState * state) override;

private:
    void RenderLightVolume(Renderer * renderer, PipeState * state, const LightCommand * command, uint shadow);
};