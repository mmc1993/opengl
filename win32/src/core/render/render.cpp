#include "render.h"
#include "../asset/shader.h"
#include "../object/camera.h"

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

void Render::PostCommand(QueueType queue, const Command & command)
{
    assert(command.mCallFn != nullptr);
	_renderQueue.at(queue).push_back(command);
}

Render::Matrix & Render::GetMatrix()
{
	return _matrix;
}

void Render::RenderOnce()
{
    for (auto & camera : _cameras)
    {
		camera.mCamera->Apply();
		OnRenderCamera(camera);
		camera.mCamera->Free();
    }
	for (auto & queue : _renderQueue)
	{
		queue.clear();
	}
}

void Render::OnRenderCamera(Camera & camera)
{
    for (auto & queue : _renderQueue)
    {
        for (auto & command : queue)
        {
            if (command.mCameraID == camera.mID)
            {
                command.mCallFn();
            }
        }
    }
}

