#pragma once

#include "pipe.h"

//  ������Ⱦ�ܵ�
class PipeForward : public Pipe {
public:
    virtual void OnAdd(Renderer * renderer, PipeState * state) override;
    virtual void OnDel(Renderer * renderer, PipeState * state) override;
    virtual void OnUpdate(Renderer * renderer, PipeState * state) override;

private:
    void PackUBO(Renderer * renderer, PipeState * state);
    void BindUBO(Renderer * renderer, PipeState * state);
};