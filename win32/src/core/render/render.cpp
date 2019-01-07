#include "render.h"
#include "../asset/shader.h"
#include "../object/camera.h"

void Render::CommandTransform::operator()()
{
    if (mIsPush)
    {
        glPushMatrix();
        glMultMatrixf(&(*mMatrix)[0][0]);
    }
    else
    {
        glPopMatrix();
    }
}


Render::Render()
{
    _renderQueue.resize(QueueType::MAX);
}

Render::~Render()
{ }

void Render::AddCamera(::Camera * camera, size_t id)
{
    assert(std::count(_cameras.begin(), _cameras.end(), id) == 0);
    auto it = std::lower_bound(
        _cameras.begin(), 
        _cameras.end(), id);
    _cameras.insert(it, Camera(camera, id));
}

void Render::DelCamera(size_t id)
{
    auto it = std::find(_cameras.begin(), _cameras.end(), id);
    if (it != _cameras.end())
    {
        _cameras.erase(it);
    }
}

void Render::PostCommand(const Command & command)
{
    assert(command.mCallFn != nullptr);
    if (command.mType == CommandType::kRENDER)
    {
        assert(command.mShader != nullptr);
        _renderQueue.at(command.mShader->GetInfo().mQueueType).push_back(command);
    }
    else
    {
        //_renderQueue.at(QueueType::kNONE).push_back(command);
    }
}

void Render::DoRender()
{
    for (auto & camera : _cameras)
    {
        RenderObjects(camera);
    }
}

void Render::RenderObjects(Camera & camera)
{
    camera.mCamera->Apply();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    for (auto & queue : _renderQueue)
    {
        for (auto & command : queue)
        {
            if (camera.mID == command.mCameraID)
            {
                command.mCallFn();
            }
        }
    }
    glFlush();
}

