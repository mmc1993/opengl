#include "render.h"
#include "../mmc.h"
#include "../asset/mesh.h"
#include "../asset/shader.h"
#include "../asset/material.h"
#include "../object/camera.h"
#include "../tools/debug_tool.h"
#include "../component/light.h"

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

void Render::AddLight(Light * light)
{
	_lights.push_back(light);
}

void Render::DelLight(Light * light)
{
	auto it = std::find(_lights.begin(), _lights.end(), light);
	if (it != _lights.end()) { _lights.erase(it); }
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
		mmc::mRender.GetMatrix().Identity(Render::Matrix::kMODEL);
		mmc::mRender.GetMatrix().Identity(Render::Matrix::kPROJECT);
		mmc::mRender.GetMatrix().Mul(Render::Matrix::kPROJECT, camera->GetProject());
	}
	else
	{
		_renderInfo.mCamera = nullptr;
		mmc::mRender.GetMatrix().Pop(Render::Matrix::kPROJECT);
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

void Render::RenderMesh()
{
	assert(_renderInfo.mMesh != nullptr);
	assert(_renderInfo.mShader != nullptr);
	assert(_renderInfo.mCamera != nullptr);
	
	_renderInfo.mShader->SetUniform("m_", GetM());
	_renderInfo.mShader->SetUniform("mv_", GetMV());
	_renderInfo.mShader->SetUniform("mvp_", GetMVP());
	_renderInfo.mShader->SetUniform("camera_pos_", _renderInfo.mCamera->GetPos());
	_renderInfo.mShader->SetUniform("camera_eye_", _renderInfo.mCamera->GetPos());

	//	TODO mmc
	//	强制单个光源
	auto light = _lights.at(0);
	auto lightPos = light->GetOwner()->GetTransform()->GetMatrixFromRoot() * glm::vec4(0, 0, 0, 1);
	_renderInfo.mShader->SetUniform("light_.mAmbient", light->mAmbient);
	_renderInfo.mShader->SetUniform("light_.mDiffuse", light->mDiffuse);
	_renderInfo.mShader->SetUniform("light_.mSpecular", light->mSpecular);
	_renderInfo.mShader->SetUniform("light_.mPosition", glm::vec3(lightPos));
	
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

const glm::mat4 & Render::GetM() const
{
	return _matrix.GetM();
}

glm::mat4 Render::GetMV() const
{
	return _renderInfo.mCamera->GetView() * GetM();
}

glm::mat4 Render::GetMVP() const
{
	return _renderInfo.mCamera->GetProject() * GetMV();
}

void Render::CommandTransform::Post(size_t cameraID, const glm::mat4 & mat)
{
	Command command;
	command.mCameraID = cameraID;
	command.mCallFn = [mat]() {
		mmc::mRender.GetMatrix().Push(Render::Matrix::kMODEL);
		mmc::mRender.GetMatrix().Mul(Render::Matrix::kMODEL, mat);
	};
	mmc::mRender.PostCommand(command);
}

void Render::CommandTransform::Free(size_t cameraID)
{
	Command command;
	command.mCameraID = cameraID;
	command.mCallFn = []() {
		mmc::mRender.GetMatrix().Pop(Render::Matrix::kMODEL);
	};
	mmc::mRender.PostCommand(command);
}
