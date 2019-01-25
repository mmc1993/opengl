#include "render.h"
#include "../mmc.h"
#include "../third/sformat.h"
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

void Render::BindLight()
{
	auto directNum = 0, pointNum = 0, spotNum = 0;
	for (auto i = 0; i != _lights.size(); ++i)
	{
		switch (_lights.at(i)->GetType())
		{
		case Light::kDIRECT:
			{
				auto direct = reinterpret_cast<LightDirect *>(_lights.at(i));
				_renderInfo.mShader->SetUniform(SFormat("light_.mDirects[{0}].mNormal", directNum), direct->mNormal);
				_renderInfo.mShader->SetUniform(SFormat("light_.mDirects[{0}].mAmbient", directNum), direct->mAmbient);
				_renderInfo.mShader->SetUniform(SFormat("light_.mDirects[{0}].mDiffuse", directNum), direct->mDiffuse);
				_renderInfo.mShader->SetUniform(SFormat("light_.mDirects[{0}].mSpecular", directNum), direct->mSpecular);
				++directNum;
			}
			break;
		case Light::kPOINT:
			{
				auto point = reinterpret_cast<LightPoint *>(_lights.at(i));
				auto position = point->GetOwner()->GetTransform()->GetWorldPosition();
				_renderInfo.mShader->SetUniform(SFormat("light_.mPoints[{0}].mK0", pointNum), point->mK0);
				_renderInfo.mShader->SetUniform(SFormat("light_.mPoints[{0}].mK1", pointNum), point->mK1);
				_renderInfo.mShader->SetUniform(SFormat("light_.mPoints[{0}].mK2", pointNum), point->mK2);
				_renderInfo.mShader->SetUniform(SFormat("light_.mPoints[{0}].mPosition", pointNum), position);
				_renderInfo.mShader->SetUniform(SFormat("light_.mPoints[{0}].mAmbient", pointNum), point->mAmbient);
				_renderInfo.mShader->SetUniform(SFormat("light_.mPoints[{0}].mDiffuse", pointNum), point->mDiffuse);
				_renderInfo.mShader->SetUniform(SFormat("light_.mPoints[{0}].mSpecular", pointNum), point->mSpecular);
				++pointNum;
			}
			break;
		case Light::kSPOT:
			{
				auto spot = reinterpret_cast<LightSpot *>(_lights.at(i));
				auto position = spot->GetOwner()->GetTransform()->GetWorldPosition();
				auto normal = spot->GetOwner()->GetTransform()->ApplyRotate(spot->mNormal);
				_renderInfo.mShader->SetUniform(SFormat("light_.mSpots[{0}].mK0", spotNum), spot->mK0);
				_renderInfo.mShader->SetUniform(SFormat("light_.mSpots[{0}].mK1", spotNum), spot->mK1);
				_renderInfo.mShader->SetUniform(SFormat("light_.mSpots[{0}].mK2", spotNum), spot->mK2);
				_renderInfo.mShader->SetUniform(SFormat("light_.mSpots[{0}].mNormal", spotNum), normal);
				_renderInfo.mShader->SetUniform(SFormat("light_.mSpots[{0}].mPosition", spotNum), position);
				_renderInfo.mShader->SetUniform(SFormat("light_.mSpots[{0}].mInCone", spotNum), spot->mInCone);
				_renderInfo.mShader->SetUniform(SFormat("light_.mSpots[{0}].mOutCone", spotNum), spot->mOutCone);
				_renderInfo.mShader->SetUniform(SFormat("light_.mSpots[{0}].mAmbient", spotNum), spot->mAmbient);
				_renderInfo.mShader->SetUniform(SFormat("light_.mSpots[{0}].mDiffuse", spotNum), spot->mDiffuse);
				_renderInfo.mShader->SetUniform(SFormat("light_.mSpots[{0}].mSpecular", spotNum), spot->mSpecular);
				++spotNum;
			}
			break;
		}
	}
	_renderInfo.mShader->SetUniform("light_.mDirectNum", directNum);
	_renderInfo.mShader->SetUniform("light_.mPointNum", pointNum);
	_renderInfo.mShader->SetUniform("light_.mSpotNum", spotNum);
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

void Render::RenderMeshDebug(size_t count)
{
	assert(_renderInfo.mShader != nullptr);
	assert(_renderInfo.mCamera != nullptr);
	BindLight();
	_renderInfo.mShader->SetUniform("matrix_n_", GetMatrixN());
	_renderInfo.mShader->SetUniform("matrix_m_", GetMatrixM());
	_renderInfo.mShader->SetUniform("matrix_mv_", GetMatrixMV());
	_renderInfo.mShader->SetUniform("matrix_mvp_", GetMatrixMVP());
	_renderInfo.mShader->SetUniform("camera_pos_", _renderInfo.mCamera->GetPos());
	_renderInfo.mShader->SetUniform("camera_eye_", _renderInfo.mCamera->GetPos());
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
}

void Render::RenderMesh()
{
	assert(_renderInfo.mMesh != nullptr);
	assert(_renderInfo.mShader != nullptr);
	assert(_renderInfo.mCamera != nullptr);
	BindLight();
	_renderInfo.mShader->SetUniform("matrix_n_", GetMatrixN());
	_renderInfo.mShader->SetUniform("matrix_m_", GetMatrixM());
	_renderInfo.mShader->SetUniform("matrix_mv_", GetMatrixMV());
	_renderInfo.mShader->SetUniform("matrix_mvp_", GetMatrixMVP());
	_renderInfo.mShader->SetUniform("camera_pos_", _renderInfo.mCamera->GetPos());
	_renderInfo.mShader->SetUniform("camera_eye_", _renderInfo.mCamera->GetPos());
	glDrawElements(GL_TRIANGLES, _renderInfo.mMesh->GetIndices().size(), GL_UNSIGNED_INT, 0);
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

const glm::mat4 & Render::GetMatrixM() const
{
	return _matrix.GetM();
}

glm::mat4 Render::GetMatrixMV() const
{
	return _renderInfo.mCamera->GetView() * GetMatrixM();
}

glm::mat4 Render::GetMatrixMVP() const
{
	return _renderInfo.mCamera->GetProject() * GetMatrixMV();
}

glm::mat3 Render::GetMatrixN() const
{
	return glm::mat3(glm::transpose(glm::inverse(GetMatrixM())));
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
