#include "render.h"
#include "../mmc.h"
#include "../asset/mesh.h"
#include "../asset/shader.h"
#include "../asset/material.h"
#include "../object/camera.h"
#include "../tools/debug_tool.h"

Render::Render()
{ }

Render::~Render()
{ }

Camera * Render::GetCamera(size_t id)
{
	auto it = std::find(_cameraInfos.begin(), _cameraInfos.end(), id);
	return it != _cameraInfos.end() ? it->mCamera : nullptr;
}

void Render::AddCamera(size_t id, Camera * camera)
{
    assert(std::count(_cameraInfos.begin(), _cameraInfos.end(), id) == 0);
    auto it = std::lower_bound(
        _cameraInfos.begin(), 
        _cameraInfos.end(), id);
    _cameraInfos.insert(it, CameraInfo(camera, id));
}

void Render::DelCamera(size_t id)
{
    auto it = std::find(_cameraInfos.begin(), _cameraInfos.end(), id);
    if (it != _cameraInfos.end())
    {
        _cameraInfos.erase(it);
    }
}

void Render::Bind(Mesh * mesh)
{
	if (mesh != nullptr)
	{
		_renderInfo.mMesh = mesh;
		glBindVertexArray(_renderInfo.mMesh->GetGLID());
	}
	else
	{
		_renderInfo.mMesh = nullptr;
		glBindVertexArray(0);
	}
}

void Render::Bind(Shader * shader)
{
	if (shader != nullptr)
	{
		_renderInfo.mShader = shader;
		glUseProgram(_renderInfo.mShader->GetGLID());
	}
	else
	{
		_renderInfo.mShader = nullptr;
		glUseProgram(0);
	}
}

void Render::Bind(Camera * camera)
{
	if (camera != nullptr)
	{
		_renderInfo.mCamera = camera;
		mmc::mRender.GetMatrix().Identity(Render::Matrix::kPROJECT);
		mmc::mRender.GetMatrix().Identity(Render::Matrix::kMODELVIEW);
		mmc::mRender.GetMatrix().Mul(Render::Matrix::kPROJECT, camera->GetProject());
		mmc::mRender.GetMatrix().Mul(Render::Matrix::kMODELVIEW, camera->GetModelView());
	}
	else
	{
		_renderInfo.mCamera = nullptr;
		mmc::mRender.GetMatrix().Pop(Render::Matrix::kPROJECT);
		mmc::mRender.GetMatrix().Pop(Render::Matrix::kMODELVIEW);
	}
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

void Render::RenderMesh()
{
	assert(_renderInfo.mMesh != nullptr);
	assert(_renderInfo.mShader != nullptr);
	assert(_renderInfo.mCamera != nullptr);
	assert(_renderInfo.mMaterial != nullptr);
	_renderInfo.mShader->SetUniform("mvp_", GetMatrix().GetMVP());
	_renderInfo.mShader->SetUniform("mv_", GetMatrix().GetMV());
	_renderInfo.mShader->SetUniform("camera_pos_", _renderInfo.mCamera->GetPos());
	_renderInfo.mShader->SetUniform("camera_eye_", _renderInfo.mCamera->GetPos());
	glDrawArrays(GL_TRIANGLES, 0, _renderInfo.mMesh->GetVertexs().size());
}

void Render::RenderOnce()
{
    for (auto & camera : _cameraInfos)
    {
		Bind(camera.mCamera);
		OnRenderCamera(camera);
		Bind((Camera *)nullptr);
    }
	_commands.clear();
}

void Render::OnRenderCamera(CameraInfo & camera)
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
