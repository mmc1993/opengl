#include "render.h"
#include "../mmc.h"
#include "../third/sformat.h"
#include "../asset/shader.h"
#include "../component/camera.h"
#include "../tools/debug_tool.h"
#include "../component/light.h"
#include "../component/skybox.h"
#include "../component/transform.h"

Render::Render()
{ }

Render::~Render()
{ }

Camera * Render::GetCamera(size_t id)
{
	auto fn = [id](const CameraInfo & info)	{ return info.mID == id; };
	auto it = std::find_if(_cameraInfos.begin(), _cameraInfos.end(), fn);
	return it != _cameraInfos.end() ? it->mCamera : nullptr;
}

void Render::AddCamera(size_t idx, Camera * camera, size_t id)
{
	assert(id == (size_t)-1 || GetCamera(id) == nullptr);
	auto fn = [idx](const CameraInfo & info) { return idx < info.mIdx; };
	auto it = std::find_if(_cameraInfos.begin(), _cameraInfos.end(), fn);
	_cameraInfos.insert(it, CameraInfo(camera, idx, id));
}

void Render::DelCamera(size_t idx)
{
	auto fn = [idx](const CameraInfo & info) { return info.mIdx == idx; };
	auto it = std::remove_if(_cameraInfos.begin(), _cameraInfos.end(), fn);
	while (it != _cameraInfos.end())
	{ DelCamera(it->mCamera); }
}

void Render::DelCamera(Camera * camera)
{
	auto fn = [camera](const CameraInfo & info) { return info.mCamera == camera; };
	auto it = std::find_if(_cameraInfos.begin(), _cameraInfos.end(), fn);
	if (it != _cameraInfos.end()) _cameraInfos.erase(it);
	delete camera;
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
				if (direct->GetShadowTex() != nullptr)
				{
					BindTexture(SFormat("light_.mDirect{0}ShadowTex", directNum), direct->GetShadowTex());

					_renderInfo.mShader->SetUniform(SFormat("light_.mDirect{0}ShadowMat", directNum), direct->GetShadowMat());
				}
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
				if (point->GetShadowTex() != nullptr)
				{
					BindTexture(SFormat("light_.mPoint{0}ShadowTex", spotNum), point->GetShadowTex());
					_renderInfo.mShader->SetUniform(SFormat("light_.mPoint{0}ShadowMat0", pointNum), point->GetShadowMat(0));
					_renderInfo.mShader->SetUniform(SFormat("light_.mPoint{0}ShadowMat1", pointNum), point->GetShadowMat(1));
					_renderInfo.mShader->SetUniform(SFormat("light_.mPoint{0}ShadowMat2", pointNum), point->GetShadowMat(2));
					_renderInfo.mShader->SetUniform(SFormat("light_.mPoint{0}ShadowMat3", pointNum), point->GetShadowMat(3));
					_renderInfo.mShader->SetUniform(SFormat("light_.mPoint{0}ShadowMat4", pointNum), point->GetShadowMat(4));
					_renderInfo.mShader->SetUniform(SFormat("light_.mPoint{0}ShadowMat5", pointNum), point->GetShadowMat(5));
				}
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
				if (spot->GetShadowTex() != nullptr)
				{
					BindTexture(SFormat("light_.mSpot{0}ShadowTex", spotNum), spot->GetShadowTex());

					_renderInfo.mShader->SetUniform(SFormat("light_.mSpot{0}ShadowMat", spotNum), spot->GetShadowMat());
				}
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

void Render::Bind(Shader * shader)
{
	if (shader != nullptr)
	{
		_renderInfo.mTexCount = 0;
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
		mmc::mRender.GetMatrix().Identity(Render::Matrix::kVIEW);
		mmc::mRender.GetMatrix().Identity(Render::Matrix::kPROJECT);
		mmc::mRender.GetMatrix().Mul(Render::Matrix::kVIEW, camera->GetView());
		mmc::mRender.GetMatrix().Mul(Render::Matrix::kPROJECT, camera->GetProject());
		glViewport((int)camera->GetViewport().x, (int)camera->GetViewport().y, 
				   (int)camera->GetViewport().z, (int)camera->GetViewport().w);
	}
	else
	{
		_renderInfo.mCamera = nullptr;
		mmc::mRender.GetMatrix().Pop(Render::Matrix::kMODEL);
		mmc::mRender.GetMatrix().Pop(Render::Matrix::kVIEW);
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

void Render::RenderVexInst(GLuint vao, size_t count, size_t instanceCount)
{
	RenderVAO(vao);
	glDrawArraysInstanced(GL_TRIANGLES, 0, count, instanceCount);
}

void Render::RenderIdxInst(GLuint vao, size_t count, size_t instanceCount)
{
	RenderVAO(vao);
	glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr, instanceCount);
}

void Render::RenderVex(GLuint vao, size_t count)
{
	RenderVAO(vao);
	glDrawArrays(GL_TRIANGLES, 0, count);
}

void Render::RenderIdx(GLuint vao, size_t count)
{
	RenderVAO(vao);
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
}

void Render::OnRenderCamera(CameraInfo * camera)
{
	//	camera != nullptr 的时候，表示本次渲染是多相机渲染，
	//	而多相机渲染需要保留每个相机渲染的结果，因此不需要glClear。
	if (camera == nullptr)
	{
		glClear(GL_COLOR_BUFFER_BIT |
				GL_DEPTH_BUFFER_BIT |
				GL_STENCIL_BUFFER_BIT);
	}
	for (auto & command : _commands)
	{
		if (camera == nullptr ||
			camera != nullptr && 
			camera->mIdx == command.mCameraIdx)
		{
			command.mCallFn();
		}
	}
}

void Render::RenderOnce()
{
	glClear(GL_COLOR_BUFFER_BIT |
			GL_DEPTH_BUFFER_BIT |
			GL_STENCIL_BUFFER_BIT);
	for (auto light : _lights)
	{
		light->DrawShadow();
	}
	for (auto & camera : _cameraInfos)
	{
		Bind(camera.mCamera);
		OnRenderCamera(&camera);
		Bind((Camera *)nullptr);
	}
	_commands.clear();
}

void Render::BindTexture(const std::string & key, const Texture & val)
{
	_renderInfo.mShader->SetUniform(key, val, _renderInfo.mTexCount++);
}

void Render::BindTexture(const std::string & key, const Bitmap * val)
{
	_renderInfo.mShader->SetUniform(key, val, _renderInfo.mTexCount++);
}

void Render::BindTexture(const std::string & key, const BitmapCube * val)
{
	_renderInfo.mShader->SetUniform(key, val, _renderInfo.mTexCount++);
}

glm::mat4 Render::GetMatrixMVP() const
{
	return _matrix.GetP() * _matrix.GetV() * _matrix.GetM();
}

glm::mat4 Render::GetMatrixMV() const
{
	return _matrix.GetV() * _matrix.GetM();
}

glm::mat3 Render::GetMatrixN() const
{
	return glm::mat3(glm::transpose(glm::inverse(_matrix.GetM())));
}

void Render::RenderVAO(GLuint vao)
{
	assert(_renderInfo.mShader != nullptr);
	BindLight();
	glBindVertexArray(vao);
	auto skybox = mmc::mRoot.GetComponent<Skybox>();
	if (skybox != nullptr)
	{
		BindTexture("skybox_", skybox->GetBitmapCube());
	}
	_renderInfo.mShader->SetUniform("matrix_n_", GetMatrixN());
	_renderInfo.mShader->SetUniform("matrix_p_", _matrix.GetP());
	_renderInfo.mShader->SetUniform("matrix_v_", _matrix.GetV());
	_renderInfo.mShader->SetUniform("matrix_m_", _matrix.GetM());
	_renderInfo.mShader->SetUniform("matrix_mv_", GetMatrixMV());
	_renderInfo.mShader->SetUniform("matrix_mvp_", GetMatrixMVP());
	if (_renderInfo.mCamera != nullptr)
	{
		_renderInfo.mShader->SetUniform("camera_pos_", _renderInfo.mCamera->GetPos());
		_renderInfo.mShader->SetUniform("camera_eye_", _renderInfo.mCamera->GetPos());
	}
}

void Render::CommandTransform::Post(size_t cameraIdx, const glm::mat4 & mat)
{
	Command command;
	command.mCameraIdx = cameraIdx;
	command.mCallFn = [mat]() {
		mmc::mRender.GetMatrix().Push(Render::Matrix::kMODEL);
		mmc::mRender.GetMatrix().Mul(Render::Matrix::kMODEL, mat);
	};
	mmc::mRender.PostCommand(command);
}

void Render::CommandTransform::Free(size_t cameraIdx)
{
	Command command;
	command.mCameraIdx = cameraIdx;
	command.mCallFn = []() {
		mmc::mRender.GetMatrix().Pop(Render::Matrix::kMODEL);
	};
	mmc::mRender.PostCommand(command);
}
