#include "render.h"
#include "../mmc.h"
#include "../asset/shader.h"
#include "../object/camera.h"

Render::Render()
{ }

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
	_commands.push_back(command);
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
	_commands.clear();
}

void Render::OnRenderCamera(Camera & camera)
{
    for (auto & command : _commands)
    {
        if (command.mCameraID == camera.mID)
        {
			command.mCallFn();
		}
    }
}

void Render::CommandTransform::Post(size_t cameraID, const glm::mat4 & mat)
{
	Command command;
	command.mCameraID = cameraID;
	command.mCallFn = [mat]() {
		mmc::mRender.GetMatrix().Push(Render::Matrix::kMODELVIEW);
		mmc::mRender.GetMatrix().Mul(Render::Matrix::kMODELVIEW, mat);
	};
	mmc::mRender.PostCommand(command);
}

void Render::CommandTransform::Free(size_t cameraID)
{
	Command command;
	command.mCameraID = cameraID;
	command.mCallFn = []() {
		mmc::mRender.GetMatrix().Pop(Render::Matrix::kMODELVIEW);
	};
	mmc::mRender.PostCommand(command);
}
