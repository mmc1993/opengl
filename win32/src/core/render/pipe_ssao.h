#pragma once

#include "pipe.h"

//  SSAO‰÷»æπ‹µ¿
class PipeSSAO : public Pipe {
public:
    virtual void OnAdd(Renderer * renderer, PipeState * state) override;
    virtual void OnDel(Renderer * renderer, PipeState * state) override;
    virtual void OnUpdate(Renderer * renderer, PipeState * state) override;

private:
    GLMesh *    _screen;
    GLProgram * _program;
};