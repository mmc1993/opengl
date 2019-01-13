#include "render.h"
#include "../mmc.h"
#include "../asset/shader.h"
#include "../object/camera.h"
#include "../tools/debug_tool.h"

Render::Render()
{ }

Render::~Render()
{ }

void Render::AddCamera(size_t id, ::Camera * camera)
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

void Render::Bind(Mesh * mesh)
{
	_renderInfo.mMesh = mesh;
}

void Render::Bind(Shader * shader)
{
	_renderInfo.mShader = shader;
}

void Render::Bind(::Camera * camera)
{
	_renderInfo.mCamera = camera;
}

void Render::Bind(Material * material)
{
	_renderInfo.mMaterial = material;
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

void Render::RenderMesh(size_t count)
{
	assert(_renderInfo.mShader != nullptr);
	_renderInfo.mShader->SetUniform("nmvp_", GetMatrix().GetNMat());
	_renderInfo.mShader->SetUniform("mvp_", GetMatrix().GetMVP());
	_renderInfo.mShader->SetUniform("mv_", GetMatrix().GetMV());
	_renderInfo.mShader->SetUniform("camera_pos_", _renderInfo.mCamera->GetPos());
	_renderInfo.mShader->SetUniform("camera_eye_", _renderInfo.mCamera->GetEye());
	glDrawArrays(GL_TRIANGLES, 0, count);
}

void Render::RenderOnce()
{
    for (auto & camera : _cameras)
    {
		Bind(camera.mCamera);
		camera.mCamera->Bind();
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
