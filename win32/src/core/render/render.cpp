#include "render.h"
#include "../mmc.h"
#include "../asset/shader.h"
#include "../asset/material.h"
#include "../component/camera.h"
#include "../tools/debug_tool.h"
#include "../component/light.h"
#include "../component/skybox.h"
#include "../component/transform.h"

Render::Render()
{ }

Render::~Render()
{ }

void Render::AddCamera(Camera * camera, size_t flag, size_t order)
{
	assert(order == (size_t)-1 || GetCamera(order) == nullptr);
	auto fn = [order](const CameraInfo & info) { return order <= info.mOrder; };
	auto it = std::find_if(_cameraInfos.begin(), _cameraInfos.end(), fn);
	_cameraInfos.insert(it, CameraInfo(camera, flag, order));
}

Camera * Render::GetCamera(size_t order)
{
	auto fn = [order](const CameraInfo & info) { return info.mOrder == order; };
	auto it = std::find_if(_cameraInfos.begin(), _cameraInfos.end(), fn);
	return it != _cameraInfos.end() ? it->mCamera : nullptr;
}

void Render::DelCamera(Camera * camera)
{
	auto fn = [camera](const CameraInfo & info) { return info.mCamera == camera; };
	auto it = std::find_if(_cameraInfos.begin(), _cameraInfos.end(), fn);
	if (it != _cameraInfos.end()) _cameraInfos.erase(it);
	delete camera;
}

void Render::DelCamera(size_t order)
{
	auto fn = [order](const CameraInfo & info) { return info.mOrder == order; };
	auto it = std::remove_if(_cameraInfos.begin(), _cameraInfos.end(), fn);
	if (it != _cameraInfos.end())
	{ DelCamera(it->mCamera); }
}

void Render::AddLight(Light * light)
{
	_lights.push_back(light);
}

void Render::DelLight(Light * light)
{
	auto it = std::remove(_lights.begin(), _lights.end(), light);
	if (it != _lights.end()) { _lights.erase(it); }
}

void Render::PostCommand(const Shader * shader, const RenderCommand & command)
{
    for (const auto & pass : shader->GetPasss())
    {
        auto cmd = command;
        cmd.mPass = &pass;
        switch (cmd.mPass->mRenderType)
        {
        case RenderTypeEnum::kSHADOW:
            _shadowCommands.at(cmd.mPass->mRenderQueue).push_back(cmd); break;
        case RenderTypeEnum::kFORWARD:
            _forwardCommands.at(cmd.mPass->mRenderQueue).push_back(cmd); break;
        case RenderTypeEnum::kDEFERRED:
            _deferredCommands.at(cmd.mPass->mRenderQueue).push_back(cmd); break;
        }
    }
}

RenderMatrix & Render::GetMatrix()
{
	return _matrix;
}

void Render::RenderOnce()
{
	_renderInfo.mVertexCount = 0;
	_renderInfo.mRenderCount = 0;
	glClear(GL_COLOR_BUFFER_BIT |
			GL_DEPTH_BUFFER_BIT |
			GL_STENCIL_BUFFER_BIT);
	//	烘培深度贴图
	std::for_each(_lights.begin(), _lights.end(), Render::OnRenderShadow);
	//	逐相机执行渲染命令
	for (auto & camera : _cameraInfos)
	{
		Bind(camera.mCamera);
		OnRenderCamera(&camera);
		Bind((Camera *)nullptr);
	}
    for (auto & queue : _shadowCommands) { queue.clear(); }
    for (auto & queue : _forwardCommands) { queue.clear(); }
    for (auto & queue : _deferredCommands) { queue.clear(); }
}

void Render::OnRenderShadow(Light * light)
{

}

void Render::OnRenderCamera(CameraInfo * camera)
{
	//  延迟渲染
	OnRenderDeferred(camera);
	//  正向渲染
	OnRenderForward(camera);
	//	后期处理
}

void Render::OnRenderForward(CameraInfo * camera)
{
	std::for_each(_forwardCommands.begin(), _forwardCommands.end(),
		std::bind(&Render::OnRenderForwardCommands, this, camera, std::placeholders::_1));
}

void Render::OnRenderDeferred(CameraInfo * camera)
{
	
}

void Render::OnRenderForwardCommands(CameraInfo * camera, const RenderQueue & commands)
{
	for (const auto & command : commands)
	{
		if (camera == nullptr || camera != nullptr &&
			(camera->mFlag & command.mCameraFlag) != 0)
		{
			if (Bind(*command.mPass))
			{
				BindFrameParam();
				BindLightParam();
			}
			BindEveryParam(command);

			for (auto i = 0; i != command.mMeshNum; ++i)
			{
				Bind(command.mMaterials[i]);

				Draw(command.mPass->mDrawType, command.mMeshs[i]);
			}
		}
	}
}

void Render::OnRenderDeferredCommands(CameraInfo * camera, const RenderQueue & commands)
{
}

void Render::BindTexture(const std::string & key, const Texture & val)
{
	Shader::SetUniform(_renderInfo.mPass->GLID, key, val, _renderInfo.mTexCount++);
}

void Render::BindTexture(const std::string & key, const Bitmap * val)
{
	Shader::SetUniform(_renderInfo.mPass->GLID, key, val, _renderInfo.mTexCount++);
}

void Render::BindTexture(const std::string & key, const BitmapCube * val)
{
	Shader::SetUniform(_renderInfo.mPass->GLID, key, val, _renderInfo.mTexCount++);
}

void Render::Bind(Camera * camera)
{
	if (camera != nullptr)
	{
		_renderInfo.mCamera = camera;
		mmc::mRender.GetMatrix().Identity(RenderMatrix::kVIEW);
		mmc::mRender.GetMatrix().Identity(RenderMatrix::kPROJ);
		mmc::mRender.GetMatrix().Mul(RenderMatrix::kVIEW, camera->GetView());
		mmc::mRender.GetMatrix().Mul(RenderMatrix::kPROJ, camera->GetProj());
		glViewport((int)camera->GetViewport().x, (int)camera->GetViewport().y,
				   (int)camera->GetViewport().z, (int)camera->GetViewport().w);
	}
	else
	{
		_renderInfo.mCamera = nullptr;
		mmc::mRender.GetMatrix().Pop(RenderMatrix::kVIEW);
		mmc::mRender.GetMatrix().Pop(RenderMatrix::kPROJ);
	}
}

bool Render::Bind(const RenderPass & pass)
{
	if (_renderInfo.mPass != &pass)
	{
		_renderInfo.mPass = &pass;

		//	开启面剔除
		if (pass.bCullFace)
		{
			glEnable(GL_CULL_FACE);
			glCullFace(pass.vCullFace);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}
		//	启用颜色混合
		if (pass.bBlend)
		{
			glEnable(GL_BLEND);
			glBlendFunc(pass.vBlendSrc, pass.vBlendDst);
		}
		else
		{
			glDisable(GL_BLEND);
		}
		//	启用深度测试
		if (pass.bDepthTest)
		{
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
		//	启用模板测试
		if (pass.bStencilTest)
		{
			glEnable(GL_STENCIL_TEST);
			glStencilMask(0xFF);
			glStencilFunc(pass.vStencilFunc, pass.vStencilRef, pass.vStencilMask);
			glStencilOp(pass.vStencilOpFail, pass.vStencilOpZFail, pass.vStencilOpZPass);
		}
		else
		{
			glDisable(GL_STENCIL_TEST);
		}
		glUseProgram(pass.GLID);
		return true;
	}
	return false;
}

void Render::Bind(const Material & material)
{
	_renderInfo.mTexCount = 0;

	for (auto i = 0; i != material.mDiffuses.size(); ++i)
	{
		BindTexture(SFormat("material_.mDiffuse{0}", i), material.mDiffuses.at(i));
	}
	if (material.mSpecular.GetBitmap() != nullptr)
	{
		BindTexture("material_.mSpecular", material.mSpecular);
	}
	if (material.mReflect.GetBitmap() != nullptr)
	{
		BindTexture("material_.mReflect", material.mReflect);
	}
	if (material.mNormal.GetBitmap() != nullptr)
	{
		BindTexture("material_.mNormal", material.mNormal);
	}
	if (material.mHeight.GetBitmap() != nullptr)
	{
		BindTexture("material_.mHeight", material.mHeight);
	}
	Shader::SetUniform(_renderInfo.mPass->GLID, "material_.mShininess", material.mShininess);
}

void Render::BindEveryParam(const RenderCommand & command)
{
	auto & matrixM			= command.mTransform;
	auto & matrixV			= _matrix.GetV();
	auto & matrixP			= _matrix.GetP();
	const auto & matrixN	= glm::transpose(glm::inverse(glm::mat3(matrixM)));
	const auto & matrixMV	= matrixV * matrixM;
	const auto & matrixMVP	= matrixP * matrixMV;
	Shader::SetUniform(_renderInfo.mPass->GLID, "matrix_n_", matrixN);
	Shader::SetUniform(_renderInfo.mPass->GLID, "matrix_m_", matrixM);
	Shader::SetUniform(_renderInfo.mPass->GLID, "matrix_v_", matrixV);
	Shader::SetUniform(_renderInfo.mPass->GLID, "matrix_p_", matrixP);
	Shader::SetUniform(_renderInfo.mPass->GLID, "matrix_mv_", matrixMV);
	Shader::SetUniform(_renderInfo.mPass->GLID, "matrix_mvp_", matrixMVP);
	Shader::SetUniform(_renderInfo.mPass->GLID, "game_time_", glfwGetTime());

	if (_renderInfo.mCamera != nullptr)
	{
		Shader::SetUniform(_renderInfo.mPass->GLID, "camera_pos_", _renderInfo.mCamera->GetPos());
		Shader::SetUniform(_renderInfo.mPass->GLID, "camera_eye_", _renderInfo.mCamera->GetEye());
	}
}

void Render::BindFrameParam()
{
	auto skybox = mmc::mRoot.GetComponent<Skybox>();
	if (skybox != nullptr)
	{
		BindTexture("skybox_", skybox->GetTexture());
	}
}

void Render::BindLightParam()
{
	auto directNum = 0, pointNum = 0, spotNum = 0;
	for (auto i = 0; i != _lights.size(); ++i)
	{
		switch (_lights.at(i)->GetType())
		{
		case Light::kDIRECT:
			{
				auto direct = reinterpret_cast<LightDirect *>(_lights.at(i));
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mDirects[{0}].mNormal", directNum), direct->mNormal);
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mDirects[{0}].mAmbient", directNum), direct->mAmbient);
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mDirects[{0}].mDiffuse", directNum), direct->mDiffuse);
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mDirects[{0}].mSpecular", directNum), direct->mSpecular);
				if (direct->GetShadowTex() != nullptr)
				{
					Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mDirects[{0}].mShadowMat", directNum), direct->GetShadowMat());

					Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mDirects[{0}].mShadowTex", directNum), direct->GetShadowTex(), _renderInfo.mTexCount++);
				}
				++directNum;
			}
			break;
		case Light::kPOINT:
			{
				auto point = reinterpret_cast<LightPoint *>(_lights.at(i));
				auto position = point->GetOwner()->GetTransform()->GetWorldPosition();
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mPoints[{0}].mK0", pointNum), point->mK0);
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mPoints[{0}].mK1", pointNum), point->mK1);
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mPoints[{0}].mK2", pointNum), point->mK2);
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mPoints[{0}].mPosition", pointNum), position);
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mPoints[{0}].mAmbient", pointNum), point->mAmbient);
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mPoints[{0}].mDiffuse", pointNum), point->mDiffuse);
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mPoints[{0}].mSpecular", pointNum), point->mSpecular);
				if (point->GetShadowTex() != nullptr)
				{
					Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mPoints[{0}].mShadowMat0", pointNum), point->GetShadowMat(0));
					Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mPoints[{0}].mShadowMat1", pointNum), point->GetShadowMat(1));
					Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mPoints[{0}].mShadowMat2", pointNum), point->GetShadowMat(2));
					Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mPoints[{0}].mShadowMat3", pointNum), point->GetShadowMat(3));
					Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mPoints[{0}].mShadowMat4", pointNum), point->GetShadowMat(4));
					Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mPoints[{0}].mShadowMat5", pointNum), point->GetShadowMat(5));
					Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mPoints[{0}].mShadowTex", pointNum), point->GetShadowTex(), _renderInfo.mTexCount++);
				}
				++pointNum;
			}
			break;
		case Light::kSPOT:
			{
				auto spot = reinterpret_cast<LightSpot *>(_lights.at(i));
				auto position = spot->GetOwner()->GetTransform()->GetWorldPosition();
				auto normal = spot->GetOwner()->GetTransform()->ApplyRotate(spot->mNormal);
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mSpots[{0}].mK0", spotNum), spot->mK0);
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mSpots[{0}].mK1", spotNum), spot->mK1);
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mSpots[{0}].mK2", spotNum), spot->mK2);
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mSpots[{0}].mNormal", spotNum), normal);
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mSpots[{0}].mPosition", spotNum), position);
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mSpots[{0}].mInCone", spotNum), spot->mInCone);
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mSpots[{0}].mOutCone", spotNum), spot->mOutCone);
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mSpots[{0}].mAmbient", spotNum), spot->mAmbient);
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mSpots[{0}].mDiffuse", spotNum), spot->mDiffuse);
				Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mSpots[{0}].mSpecular", spotNum), spot->mSpecular);
				if (spot->GetShadowTex() != nullptr)
				{
					Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mSpots[{0}].mShadowMat", spotNum), spot->GetShadowMat());

					Shader::SetUniform(_renderInfo.mPass->GLID, SFormat("light_.mSpots[{0}].mShadowTex", spotNum), spot->GetShadowTex(), _renderInfo.mTexCount++);
				}
				++spotNum;
			}
			break;
		}
	}
	Shader::SetUniform(_renderInfo.mPass->GLID, "light_.mDirectNum", directNum);
	Shader::SetUniform(_renderInfo.mPass->GLID, "light_.mPointNum", pointNum);
	Shader::SetUniform(_renderInfo.mPass->GLID, "light_.mSpotNum", spotNum);
}

void Render::Draw(DrawTypeEnum drawType, const RenderMesh & mesh)
{
	assert(mesh.mVBO != 0);
	assert(mesh.mVAO != 0);
	glBindVertexArray(mesh.mVAO);
	switch (drawType)
	{
	case DrawTypeEnum::kINSTANCE:
		{
			//	TODO, 暂不实现
		}
		break;
	case DrawTypeEnum::kVERTEX:
		{
			_renderInfo.mVertexCount += mesh.mVtxCount;
			glDrawArrays(GL_TRIANGLES, 0, mesh.mVtxCount);
		}
		break;
	case DrawTypeEnum::kINDEX:
		{
			_renderInfo.mVertexCount += mesh.mIdxCount;
			glDrawElements(GL_TRIANGLES, mesh.mIdxCount, GL_UNSIGNED_INT, nullptr);
		}
		break;
	}
	++_renderInfo.mRenderCount;
}
