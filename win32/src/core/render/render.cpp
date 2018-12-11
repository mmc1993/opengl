#include "render.h"
#include "../shader/shader.h"

Render::Render()
{
    _renderQueue.resize(RenderQueueFlag::MAX);
}

Render::~Render()
{ }

void Render::PostCommand(Shader * shader, const std::function<void()>& callfn)
{
    Command cmd;
    cmd.mShader = shader;
    cmd.mCallFn = callfn;
    _renderQueue.at(shader->GetInfo().mRenderFlag).push_back(cmd);
}

void Render::DoRender()
{
    for (auto & queue : _renderQueue)
    {
        for (auto & command : queue)
        {
            command.mCallFn();
        }
    }
}
