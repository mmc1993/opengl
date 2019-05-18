#include "render.h"
#include "../res/mesh.h"
#include "../res/shader.h"
#include "../res/material.h"
#include "../component/light.h"
#include "../component/camera.h"
#include "../component/skybox.h"
#include "../component/transform.h"
#include "../cfg/cfg_cache.h"

Render::Render()
{ }

Render::~Render()
{
    if (_uboLightForward[UBOLightForwardTypeEnum::kDIRECT] != 0 ||
        _uboLightForward[UBOLightForwardTypeEnum::kPOINT] != 0 ||
        _uboLightForward[UBOLightForwardTypeEnum::kSPOT] != 0)
    {
        ASSERT_LOG(_uboLightForward[UBOLightForwardTypeEnum::kDIRECT] != 0, "~Render _uboLightForward[UBOLightForwardTypeEnum::kDIRECT]: {0}", _uboLightForward[UBOLightForwardTypeEnum::kDIRECT]);
        ASSERT_LOG(_uboLightForward[UBOLightForwardTypeEnum::kPOINT] != 0, "~Render _uboLightForward[UBOLightForwardTypeEnum::kPOINT]: {0}", _uboLightForward[UBOLightForwardTypeEnum::kPOINT]);
        ASSERT_LOG(_uboLightForward[UBOLightForwardTypeEnum::kSPOT] != 0, "~Render _uboLightForward[UBOLightForwardTypeEnum::kSPOT]: {0}", _uboLightForward[UBOLightForwardTypeEnum::kSPOT]);
        glDeleteBuffers(3, _uboLightForward);
    }

    if (_gbuffer.mPositionTexture != 0)
    {
        ASSERT_LOG(_gbuffer.mPositionTexture != 0, "~Render _gbuffer.mPositionTexture: {0}", _gbuffer.mPositionTexture);
        ASSERT_LOG(_gbuffer.mSpecularTexture != 0, "~Render _gbuffer.mSpecularTexture: {0}", _gbuffer.mSpecularTexture);
        ASSERT_LOG(_gbuffer.mDiffuseTexture != 0, "~Render _gbuffer.mDiffuseTexture: {0}", _gbuffer.mDiffuseTexture);
        ASSERT_LOG(_gbuffer.mNormalTexture != 0, "~Render _gbuffer.mNormalTexture: {0}", _gbuffer.mNormalTexture);
        ASSERT_LOG(_gbuffer.mDepthBuffer != 0, "~Render _gbuffer.mDepthBuffer: {0}", _gbuffer.mDepthBuffer);
        glDeleteTextures(4, &_gbuffer.mPositionTexture);
        glDeleteRenderbuffers(1,&_gbuffer.mDepthBuffer);
    }

    if (_offSceneBuffer.mColorTexture != 0)
    {
        ASSERT_LOG(_offSceneBuffer.mColorTexture != 0, "~Render _offSceneBuffer.mColorTexture : {0}", _offSceneBuffer.mColorTexture);
        ASSERT_LOG(_offSceneBuffer.mDepthTexture != 0, "~Render _offSceneBuffer.mDepthTexture : {0}", _offSceneBuffer.mDepthTexture);
        glDeleteTextures(2, &_offSceneBuffer.mColorTexture);
    }
}

MatrixStack & Render::GetMatrixStack()
{
    return _matrixStack;
}

void Render::AddCamera(Camera * camera, size_t flag, size_t order)
{
    ASSERT_LOG(order == ~0 || GetCamera(order) == nullptr, "AddCamera Flag Order: {0}, {1}", flag, order);
	
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

void Render::RenderOnce()
{
    StartRender();
	for (auto & camera : _cameraInfos)
	{
		Bind(&camera);
		RenderCamera();
		Bind((CameraInfo *)nullptr);
	}
	ClearCommands();
}

void Render::PostCommand(const Shader * shader, const RenderCommand & command)
{
    switch (command.mType)
    {
    case RenderCommand::kOBJECT:
        {
            auto cmd = reinterpret_cast<const ObjectCommand &>(command);
            for (const auto & pass : shader->GetPasss())
            {
                cmd.mPass = &pass;
                switch (cmd.mPass->mRenderType)
                {
                case RenderTypeEnum::kSHADOW:
                    _shadowCommands.push_back(cmd); break;
                case RenderTypeEnum::kFORWARD:
                    _forwardQueues.at(cmd.mPass->mRenderQueue).push_back(cmd); break;
                case RenderTypeEnum::kDEFERRED:
                    _deferredQueues.at(cmd.mPass->mRenderQueue).push_back(cmd); break;
                }
            }
        }
        break;
    case RenderCommand::kLIGHT:
        {
            auto & cmd = reinterpret_cast<const LightCommand &>(command);
            _lightQueues.at(cmd.mLight->GetType()).push_back(cmd);
        }
        break;
    }
}

void Render::BakeLightDepthMap(Light * light)
{
    auto count = 0;
    _renderInfo.mPass = nullptr;
    _renderTarget[0].Start();
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
	while (light->NextDrawShadow(count++, &_renderTarget[0]))
	{
        glClear(GL_DEPTH_BUFFER_BIT);

		for (auto & command : _shadowCommands)
		{
            if (Bind(command.mPass))
            {
                Post(light);
            }
            
            Post(command.mTransform);

            for (auto i = 0; i != command.mMeshNum; ++i)
            {
                Draw(command.mPass->mDrawType, command.mMeshs[i]);
            }
		}
	}
    _renderTarget[0].Ended();
}

void Render::SortLightCommands()
{
    auto & cameraPos = _renderInfo.mCamera->mCamera->GetPos();

    _renderInfo.mCountUseLightDirect = std::min(LIMIT_LIGHT_DIRECT, _lightQueues.at(Light::Type::kDIRECT).size());

    std::sort(_lightQueues.at(Light::Type::kPOINT).begin(), 
              _lightQueues.at(Light::Type::kPOINT).end(),
              [&cameraPos](const LightCommand & command0, const LightCommand & command1)
        {
            auto diff0 = (cameraPos - command0.mLight->mPosition);
            auto diff1 = (cameraPos - command1.mLight->mPosition);
            return glm::dot(diff0, diff0) < glm::dot(diff1, diff1);
        });
    _renderInfo.mCountUseLightPoint = std::min(LIMIT_LIGHT_POINT, _lightQueues.at(Light::Type::kPOINT).size());

    std::sort(_lightQueues.at(Light::Type::kSPOT).begin(), 
              _lightQueues.at(Light::Type::kSPOT).end(),
              [&cameraPos](const LightCommand & command0, const LightCommand & command1)
        {
            auto diff0 = (cameraPos - command0.mLight->mPosition);
            auto diff1 = (cameraPos - command1.mLight->mPosition);
            return glm::dot(diff0, diff0) < glm::dot(diff1, diff1);
        });
    _renderInfo.mCountUseLightSpot = std::min(LIMIT_LIGHT_SPOT, _lightQueues.at(Light::Type::kSPOT).size());
}

void Render::BakeLightDepthMap()
{
    //  烘培阴影的时候会修改Viewport, 在这之前先保存当前的信息.
    iint viewport[4] = { 0 };
    glGetIntegerv(GL_VIEWPORT, viewport);
    //  烘培阴影
    for (auto i = 0; i != _renderInfo.mCountUseLightDirect; ++i)
    {
        BakeLightDepthMap(_lightQueues.at(Light::Type::kDIRECT).at(i).mLight);
    }
    for (auto i = 0; i != _renderInfo.mCountUseLightPoint; ++i)
    {
        BakeLightDepthMap(_lightQueues.at(Light::Type::kPOINT).at(i).mLight);
    }
    for (auto i = 0; i != _renderInfo.mCountUseLightSpot; ++i)
    {
        BakeLightDepthMap(_lightQueues.at(Light::Type::kSPOT).at(i).mLight);
    }
    //  恢复之前的Viewport
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void Render::RenderCamera()
{
    SortLightCommands();

    BakeLightDepthMap();

    //  延迟渲染
    if (!IsEmptyQueueArray(_deferredQueues))
    {
        _renderInfo.mPass = nullptr;
        RenderDeferred();
    }

    //  正向渲染
    if (!IsEmptyQueueArray(_forwardQueues))
    {
        _renderInfo.mPass = nullptr;
        RenderForward();
    }

    _renderTarget[1].Start(RenderTarget::BindType::kREAD);
    glBlitFramebuffer(
        0, 0,
        Global::Ref().RefCfgCache().At("init")->At("window", "w")->ToInt(),
        Global::Ref().RefCfgCache().At("init")->At("window", "h")->ToInt(),
        0, 0,
        Global::Ref().RefCfgCache().At("init")->At("window", "w")->ToInt(),
        Global::Ref().RefCfgCache().At("init")->At("window", "w")->ToInt(),
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    _renderTarget[1].Ended();

	//	后期处理
    _renderInfo.mPass = nullptr;
}

void Render::RenderForward()
{
    PackUBOLightForward();

    _renderTarget[1].Start();
    for (auto & commands : _forwardQueues)
    {
        RenderForwardCommands(commands);
    }
    _renderTarget[1].Ended();
}

void Render::RenderDeferred()
{
    _renderTarget[0].Start(RenderTarget::BindType::kALL);
    _renderTarget[0].BindAttachment(RenderTarget::AttachmentType::kCOLOR0, RenderTarget::TextureType::k2D, _gbuffer.mPositionTexture);
    _renderTarget[0].BindAttachment(RenderTarget::AttachmentType::kCOLOR1, RenderTarget::TextureType::k2D, _gbuffer.mSpecularTexture);
    _renderTarget[0].BindAttachment(RenderTarget::AttachmentType::kCOLOR2, RenderTarget::TextureType::k2D, _gbuffer.mDiffuseTexture);
    _renderTarget[0].BindAttachment(RenderTarget::AttachmentType::kCOLOR3, RenderTarget::TextureType::k2D, _gbuffer.mNormalTexture);
    _renderTarget[0].BindAttachment(RenderTarget::AttachmentType::kDEPTH, _gbuffer.mDepthBuffer);

    uint rtbinds[] = { RenderTarget::AttachmentType::kCOLOR0, RenderTarget::AttachmentType::kCOLOR1, 
                       RenderTarget::AttachmentType::kCOLOR2, RenderTarget::AttachmentType::kCOLOR3 };
    glDrawBuffers(4, rtbinds);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto & commands : _deferredQueues)
    {
        RenderDeferredCommands(commands);
    }

    _renderTarget[0].Start(RenderTarget::BindType::kREAD);
    _renderTarget[1].Start(RenderTarget::BindType::kDRAW);
    glBlitFramebuffer(
        0, 0,
        Global::Ref().RefCfgCache().At("init")->At("window", "w")->ToInt(),
        Global::Ref().RefCfgCache().At("init")->At("window", "h")->ToInt(),
        0, 0,
        Global::Ref().RefCfgCache().At("init")->At("window", "w")->ToInt(),
        Global::Ref().RefCfgCache().At("init")->At("window", "w")->ToInt(),
        GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    _renderTarget[0].Ended();

    for (auto i = 0u; i != _lightQueues.at(Light::Type::kDIRECT).size(); ++i)
    {
        RenderLightVolume(_lightQueues.at(Light::Type::kDIRECT).at(i), i < _renderInfo.mCountUseLightDirect);
    }

    for (auto i = 0u; i != _lightQueues.at(Light::Type::kPOINT).size(); ++i)
    {
        RenderLightVolume(_lightQueues.at(Light::Type::kPOINT).at(i), i < _renderInfo.mCountUseLightPoint);
    }

    for (auto i = 0u; i != _lightQueues.at(Light::Type::kSPOT).size(); ++i)
    {
        RenderLightVolume(_lightQueues.at(Light::Type::kSPOT).at(i), i < _renderInfo.mCountUseLightSpot);
    }

    _renderTarget[1].Ended();
}

void Render::RenderForwardCommands(const ObjectCommandQueue & commands)
{
	for (const auto & command : commands)
	{
		if ((_renderInfo.mCamera->mFlag & command.mCameraFlag) != 0)
		{
			if (Bind(command.mPass)) 
            {
                BindUBOLightForward();
            }
			
            Post(command.mTransform);
			
            for (auto i = 0; i != command.mMeshNum; ++i)
			{
                Post(command.mMaterials[i]);

				Draw(command.mPass->mDrawType, command.mMeshs[i]);
			}
		}
	}
}

void Render::RenderDeferredCommands(const ObjectCommandQueue & commands)
{
    for (auto & command : commands)
    {
        if ((_renderInfo.mCamera->mFlag & command.mCameraFlag) != 0)
        {
            Bind(command.mPass); 
            
            Post(command.mTransform);

            for (auto i = 0; i != command.mMeshNum; ++i)
            {
                Post(command.mMaterials[i]);

                Draw(command.mPass->mDrawType, command.mMeshs[i]);
            }
        }
    }
}

void Render::RenderLightVolume(const LightCommand & command, bool isRenderShadow)
{
    const auto & pass = isRenderShadow
                      ? command.mShader->GetPass(0)
                      : command.mShader->GetPass(1);
    if (Bind(&pass))
    {
        Shader::SetTexture2D(_renderInfo.mPass->GLID, UNIFORM_GBUFFER_POSIITON, _gbuffer.mPositionTexture, _renderInfo.mTexBase + 0);
        Shader::SetTexture2D(_renderInfo.mPass->GLID, UNIFORM_GBUFFER_SPECULAR, _gbuffer.mSpecularTexture, _renderInfo.mTexBase + 1);
        Shader::SetTexture2D(_renderInfo.mPass->GLID, UNIFORM_GBUFFER_DIFFUSE, _gbuffer.mDiffuseTexture, _renderInfo.mTexBase + 2);
        Shader::SetTexture2D(_renderInfo.mPass->GLID, UNIFORM_GBUFFER_NORMAL, _gbuffer.mNormalTexture, _renderInfo.mTexBase + 3);
    }

    if (isRenderShadow)
    {
        Shader::UnbindTex2D(_renderInfo.mTexBase + 4);
        Shader::UnbindTex3D(_renderInfo.mTexBase + 4);
        switch (command.mLight->GetType())
        {
        case Light::Type::kDIRECT: Shader::SetTexture2D(_renderInfo.mPass->GLID, SFormat(UNIFORM_SHADOW_MAP_DIRECT_, 0).c_str(), command.mLight->GetSMP(), _renderInfo.mTexBase + 4); break;
        case Light::Type::kPOINT: Shader::SetTexture3D(_renderInfo.mPass->GLID, SFormat(UNIFORM_SHADOW_MAP_POINT_, 0).c_str(), command.mLight->GetSMP(), _renderInfo.mTexBase + 4); break;
        case Light::Type::kSPOT: Shader::SetTexture2D(_renderInfo.mPass->GLID, SFormat(UNIFORM_SHADOW_MAP_SPOT_, 0).c_str(), command.mLight->GetSMP(), _renderInfo.mTexBase + 4); break;
        }
    }

    Post(command.mLight);
    
    Post(command.mTransform);

    Draw(pass.mDrawType, *command.mMesh);
}

void Render::PackUBOLightForward()
{
    const auto DIRECT_UBO_LEN = LightDirect::GetUBOLength();
    const auto POINT_UBO_LEN = LightPoint::GetUBOLength();
    const auto SPOT_UBO_LEN = LightSpot::GetUBOLength();

    auto offset = 0;
    glBindBuffer(GL_COPY_WRITE_BUFFER, _uboLightForward[kDIRECT]);
    for (auto i = 0u; i != _renderInfo.mCountUseLightDirect; ++i)
    {
        glBindBuffer(GL_COPY_READ_BUFFER, _lightQueues.at(Light::Type::kDIRECT).at(i).mLight->GetUBO());
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, offset, DIRECT_UBO_LEN);
        glBindBuffer(GL_COPY_READ_BUFFER, 0);
        offset += DIRECT_UBO_LEN;
    }

    offset = 0;
    glBindBuffer(GL_COPY_WRITE_BUFFER, _uboLightForward[kPOINT]);
    for (auto i = 0; i != _renderInfo.mCountUseLightPoint; ++i)
    {
        glBindBuffer(GL_COPY_READ_BUFFER, _lightQueues.at(Light::Type::kPOINT).at(i).mLight->GetUBO());
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, offset, POINT_UBO_LEN);
        glBindBuffer(GL_COPY_READ_BUFFER, 0);
        offset += POINT_UBO_LEN;
    }

    offset = 0;
    glBindBuffer(GL_COPY_WRITE_BUFFER, _uboLightForward[kSPOT]);
    for (auto i = 0; i != _renderInfo.mCountUseLightSpot; ++i)
    {
        glBindBuffer(GL_COPY_READ_BUFFER, _lightQueues.at(Light::Type::kSPOT).at(i).mLight->GetUBO());
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, offset, SPOT_UBO_LEN);
        glBindBuffer(GL_COPY_READ_BUFFER, 0);
        offset += SPOT_UBO_LEN;
    }

    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}

void Render::BindUBOLightForward()
{
    auto idirect = glGetUniformBlockIndex(_renderInfo.mPass->GLID, UBO_NAME_LIGHT_DIRECT);
    glUniformBlockBinding(_renderInfo.mPass->GLID, idirect, UniformBlockEnum::kLIGHT_DIRECT);
    glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_DIRECT, _uboLightForward[UBOLightForwardTypeEnum::kDIRECT]);

    auto ipoint = glGetUniformBlockIndex(_renderInfo.mPass->GLID, UBO_NAME_LIGHT_POINT);
    glUniformBlockBinding(_renderInfo.mPass->GLID, ipoint, UniformBlockEnum::kLIGHT_POINT);
    glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_POINT, _uboLightForward[UBOLightForwardTypeEnum::kPOINT]);

    auto ispot = glGetUniformBlockIndex(_renderInfo.mPass->GLID, UBO_NAME_LIGHT_SPOT);
    glUniformBlockBinding(_renderInfo.mPass->GLID, ispot, UniformBlockEnum::kLIGHT_SPOT);
    glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_SPOT, _uboLightForward[UBOLightForwardTypeEnum::kSPOT]);

    Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_LIGHT_COUNT_DIRECT_, _renderInfo.mCountUseLightDirect);
    Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_LIGHT_COUNT_POINT_, _renderInfo.mCountUseLightPoint);
    Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_LIGHT_COUNT_SPOT_, _renderInfo.mCountUseLightSpot);

    for (auto i = 0, directCount = 0; i != _renderInfo.mCountUseLightDirect; ++i, ++directCount)
    {
        Shader::SetTexture2D(_renderInfo.mPass->GLID, SFormat(UNIFORM_SHADOW_MAP_DIRECT_, directCount).c_str(), _lightQueues.at(Light::Type::kDIRECT).at(i).mLight->GetSMP(), _renderInfo.mTexBase++);
    }

    for (auto i = 0, pointCount = 0; i != _renderInfo.mCountUseLightPoint; ++i, ++pointCount)
    {
        Shader::SetTexture3D(_renderInfo.mPass->GLID, SFormat(UNIFORM_SHADOW_MAP_POINT_, pointCount).c_str(), _lightQueues.at(Light::Type::kPOINT).at(i).mLight->GetSMP(), _renderInfo.mTexBase++);
    }

    for (auto i = 0, spotCount = 0; i != _renderInfo.mCountUseLightSpot; ++i, ++spotCount)
    {
        Shader::SetTexture2D(_renderInfo.mPass->GLID, SFormat(UNIFORM_SHADOW_MAP_SPOT_, spotCount).c_str(), _lightQueues.at(Light::Type::kSPOT).at(i).mLight->GetSMP(), _renderInfo.mTexBase++);
    }
}

template <class T, int N>
bool Render::IsEmptyQueueArray(const std::array<T, N> & ary)
{
    for (auto & queue : ary)
    {
        if (!queue.empty()) { return false; }
    }
    return true;
}

void Render::Bind(const CameraInfo * camera)
{
	if (camera != nullptr)
	{
		Global::Ref().RefRender().GetMatrixStack().Identity(MatrixStack::kVIEW);
		Global::Ref().RefRender().GetMatrixStack().Identity(MatrixStack::kPROJ);
		Global::Ref().RefRender().GetMatrixStack().Mul(MatrixStack::kVIEW, camera->mCamera->GetView());
		Global::Ref().RefRender().GetMatrixStack().Mul(MatrixStack::kPROJ, camera->mCamera->GetProj());
		glViewport((int)camera->mCamera->GetViewport().x, (int)camera->mCamera->GetViewport().y,
				   (int)camera->mCamera->GetViewport().z, (int)camera->mCamera->GetViewport().w);
        _renderInfo.mCamera = camera;
	}
	else
	{
		Global::Ref().RefRender().GetMatrixStack().Pop(MatrixStack::kVIEW);
		Global::Ref().RefRender().GetMatrixStack().Pop(MatrixStack::kPROJ);
        _renderInfo.mCamera = nullptr;
	}
}

void Render::Post(const Light * light)
{
    switch (light->GetType())
    {
    case Light::Type::kDIRECT:
        {
            auto idx = glGetUniformBlockIndex(_renderInfo.mPass->GLID, UBO_NAME_LIGHT_DIRECT);
            if (GL_INVALID_INDEX != idx)
            {
                glUniformBlockBinding(_renderInfo.mPass->GLID, idx, UniformBlockEnum::kLIGHT_DIRECT);
                glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_DIRECT, light->GetUBO());
            }
        }
        break;
    case Light::Type::kPOINT:
        {
            auto idx = glGetUniformBlockIndex(_renderInfo.mPass->GLID, UBO_NAME_LIGHT_POINT);
            if (GL_INVALID_INDEX != idx)
            {
                glUniformBlockBinding(_renderInfo.mPass->GLID, idx, UniformBlockEnum::kLIGHT_POINT);
                glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_POINT, light->GetUBO());
            }
        }
        break;
    case Light::Type::kSPOT:
        {
            auto idx = glGetUniformBlockIndex(_renderInfo.mPass->GLID, UBO_NAME_LIGHT_SPOT);
            if (GL_INVALID_INDEX != idx)
            {
                glUniformBlockBinding(_renderInfo.mPass->GLID, idx, UniformBlockEnum::kLIGHT_SPOT);
                glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_SPOT, light->GetUBO());
            }
        }
        break;
    }
    Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_LIGHT_TYPE, light->GetType());
}

void Render::StartRender()
{
    //  初始化正向渲染光源所需的UBO
    if (_uboLightForward[UBOLightForwardTypeEnum::kDIRECT] == 0)
    {
        ASSERT_LOG(_uboLightForward[UBOLightForwardTypeEnum::kDIRECT] == 0, "_uboLightForward[UBOLightForwardTypeEnum::kDIRECT]: {0}", _uboLightForward[UBOLightForwardTypeEnum::kDIRECT]);
        ASSERT_LOG(_uboLightForward[UBOLightForwardTypeEnum::kPOINT] == 0, "_uboLightForward[UBOLightForwardTypeEnum::kPOINT]: {0}", _uboLightForward[UBOLightForwardTypeEnum::kPOINT]);
        ASSERT_LOG(_uboLightForward[UBOLightForwardTypeEnum::kSPOT] == 0, "_uboLightForward[UBOLightForwardTypeEnum::kSPOT]: {0}", _uboLightForward[UBOLightForwardTypeEnum::kSPOT]);

        glGenBuffers(3, _uboLightForward);
        glBindBuffer(GL_UNIFORM_BUFFER, _uboLightForward[kDIRECT]);
        glBufferData(GL_UNIFORM_BUFFER, LightDirect::GetUBOLength() * LIMIT_LIGHT_DIRECT, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, _uboLightForward[kPOINT]);
        glBufferData(GL_UNIFORM_BUFFER, LightPoint::GetUBOLength() * LIMIT_LIGHT_POINT, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, _uboLightForward[kSPOT]);
        glBufferData(GL_UNIFORM_BUFFER, LightSpot::GetUBOLength() * LIMIT_LIGHT_SPOT, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    //  初始化 GBuffer
    if (_gbuffer.mPositionTexture == 0)
    {
        ASSERT_LOG(_gbuffer.mPositionTexture == 0, "_gbuffer.mPositionTexture: {0}", _gbuffer.mPositionTexture);
        ASSERT_LOG(_gbuffer.mSpecularTexture == 0, "_gbuffer.mSpecularTexture: {0}", _gbuffer.mSpecularTexture);
        ASSERT_LOG(_gbuffer.mDiffuseTexture == 0, "_gbuffer.mDiffuseTexture: {0}", _gbuffer.mDiffuseTexture);
        ASSERT_LOG(_gbuffer.mNormalTexture == 0, "_gbuffer.mNormalTexture: {0}", _gbuffer.mNormalTexture);
        ASSERT_LOG(_gbuffer.mDepthBuffer == 0, "_gbuffer.mDepthBuffer: {0}", _gbuffer.mDepthBuffer);

        glGenTextures(4, &_gbuffer.mPositionTexture);
        glGenRenderbuffers(1,&_gbuffer.mDepthBuffer);

        auto windowW = Global::Ref().RefCfgCache().At("init")->At("window", "w")->ToInt();
        auto windowH = Global::Ref().RefCfgCache().At("init")->At("window", "h")->ToInt();

        glBindTexture(GL_TEXTURE_2D, _gbuffer.mPositionTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowW, windowH, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, _gbuffer.mSpecularTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowW, windowH, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, _gbuffer.mDiffuseTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowW, windowH, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, _gbuffer.mNormalTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowW, windowH, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindRenderbuffer(GL_RENDERBUFFER, _gbuffer.mDepthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowW, windowH);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
    
    if (_offSceneBuffer.mColorTexture == 0)
    {
        ASSERT_LOG(_offSceneBuffer.mColorTexture == 0, "_offSceneBuffer.mColorTexture : {0}", _offSceneBuffer.mColorTexture);
        ASSERT_LOG(_offSceneBuffer.mDepthTexture == 0, "_offSceneBuffer.mDepthTexture : {0}", _offSceneBuffer.mDepthTexture);

        glGenTextures(2, &_offSceneBuffer.mColorTexture);

        auto windowW = Global::Ref().RefCfgCache().At("init")->At("window", "w")->ToInt();
        auto windowH = Global::Ref().RefCfgCache().At("init")->At("window", "h")->ToInt();

        glBindTexture(GL_TEXTURE_2D, _offSceneBuffer.mColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowW, windowH, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, _offSceneBuffer.mDepthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowW, windowH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        _renderTarget[1].Start();
        _renderTarget[1].BindAttachment(RenderTarget::AttachmentType::kCOLOR0, RenderTarget::TextureType::k2D, _offSceneBuffer.mColorTexture);
        _renderTarget[1].BindAttachment(RenderTarget::AttachmentType::kDEPTH, RenderTarget::TextureType::k2D, _offSceneBuffer.mDepthTexture);
        _renderTarget[1].Ended();
    }

    _renderInfo.mVertexCount = 0;
    _renderInfo.mRenderCount = 0;

    _renderTarget[1].Start();
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);
    _renderTarget[1].Ended();
}

bool Render::Bind(const Pass * pass)
{
	if (_renderInfo.mPass != pass)
	{
		_renderInfo.mPass = pass;
        _renderInfo.mTexBase = 0;

		//	开启面剔除
		if (pass->bCullFace)
		{
			glEnable(GL_CULL_FACE);
			glCullFace(pass->vCullFace);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}
		//	启用颜色混合
		if (pass->bBlend)
		{
			glEnable(GL_BLEND);
			glBlendFunc(pass->vBlendSrc, pass->vBlendDst);
		}
		else
		{
			glDisable(GL_BLEND);
		}
		//	启用深度测试
		if (pass->bDepthTest)
		{
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
		//	启用模板测试
		if (pass->bStencilTest)
		{
			glEnable(GL_STENCIL_TEST);
			glStencilMask(0xFF);
			glStencilFunc(pass->vStencilFunc, pass->vStencilRef, pass->vStencilMask);
			glStencilOp(pass->vStencilOpFail, pass->vStencilOpZFail, pass->vStencilOpZPass);
		}
		else
		{
			glDisable(GL_STENCIL_TEST);
		}
		glUseProgram(pass->GLID);
		return true;
	}
	return false;
}

void Render::Post(const Material & material)
{
    auto texIndex = _renderInfo.mTexBase;
	for (auto i = 0; i != material.mDiffuses.size(); ++i)
	{
        Shader::SetUniform(_renderInfo.mPass->GLID, SFormat(UNIFORM_MATERIAL_DIFFUSE, i).c_str(), material.mDiffuses.at(i), texIndex++);
	}
	if (material.mSpecular != nullptr)
	{
        Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATERIAL_SPECULAR, material.mSpecular, texIndex++);
	}
	if (material.mReflect != nullptr)
	{
        Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATERIAL_REFLECT, material.mReflect, texIndex++);
	}
	if (material.mNormal != nullptr)
	{
        Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATERIAL_NORMAL, material.mNormal, texIndex++);
	}
	if (material.mHeight != nullptr)
	{
        Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATERIAL_HEIGHT, material.mHeight, texIndex++);
	}
    Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATERIAL_SHININESS, material.mShininess);
}

void Render::ClearCommands()
{
	_shadowCommands.clear();
    for (auto & queue : _lightQueues) { queue.clear(); }
	for (auto & queue : _forwardQueues) { queue.clear(); }
	for (auto & queue : _deferredQueues) { queue.clear(); }
}

void Render::Post(const glm::mat4 & transform)
{
	auto & matrixM			= transform;
	auto & matrixV			= _matrixStack.GetV();
	auto & matrixP			= _matrixStack.GetP();
	const auto & matrixN	= glm::transpose(glm::inverse(glm::mat3(matrixM)));
	const auto & matrixMV	= matrixV * matrixM;
	const auto & matrixMVP	= matrixP * matrixMV;
	Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATRIX_N, matrixN);
	Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATRIX_M, matrixM);
	Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATRIX_V, matrixV);
	Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATRIX_P, matrixP);
	Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATRIX_MV, matrixMV);
	Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATRIX_MVP, matrixMVP);
	Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_GAME_TIME, glfwGetTime());
    if (_renderInfo.mCamera != nullptr)
    {
        Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_CAMERA_POS, _renderInfo.mCamera->mCamera->GetPos());
        Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_CAMERA_EYE, _renderInfo.mCamera->mCamera->GetEye());
    }
}

void Render::Draw(DrawTypeEnum drawType, const Mesh & mesh)
{
    ASSERT_LOG(mesh.mVBO != 0, "Draw VBO Error");
    ASSERT_LOG(mesh.mVAO != 0, "Draw VAO Error");

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
