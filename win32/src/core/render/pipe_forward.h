#pragma once

#include "pipe.h"

class PipeForward : public Pipe {
public:
    virtual void OnAdd(PipeState * state);

    virtual void OnDel(PipeState * state);

    virtual void OnUpdate(PipeState * state);
};