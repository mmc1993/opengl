#include "render.h"
#include "../res/mesh.h"
#include "../res/shader.h"
#include "../res/material.h"
#include "../tools/glsl_tool.h"
#include "../tools/debug_tool.h"
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
        assert(_uboLightForward[UBOLightForwardTypeEnum::kDIRECT] != 0);
        assert(_uboLightForward[UBOLightForwardTypeEnum::kPOINT] != 0);
        assert(_uboLightForward[UBOLightForwardTypeEnum::kSPOT] != 0);
        glDeleteBuffers(3, _uboLightForward);
    }

    if (_gbuffer.mPositionTexture != 0)
    {
        assert(_gbuffer.mPositionTexture != 0
            && _gbuffer.mSpecularTexture != 0
            && _gbuffer.mDiffuseTexture != 0
            && _gbuffer.mNormalTexture != 0
            && _gbuffer.mDepthTexture != 0);
        glDeleteTextures(5, &_gbuffer.mPositionTexture);
    }
}

MatrixStack & Render::GetMatrixStack()
{
    return _matrixStack;
}

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

void Render::RenderOnce()
{
	_renderInfo.mVertexCount = 0;
	_renderInfo.mRenderCount = 0;
	glClear(GL_COLOR_BUFFER_BIT |
			GL_DEPTH_BUFFER_BIT |
			GL_STENCIL_BUFFER_BIT);
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
    _renderTarget.Start();
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
	while (light->NextDrawShadow(count++, &_renderTarget))
	{
        glClear(GL_DEPTH_BUFFER_BIT);

		for (auto & command : _shadowCommands)
		{
            Bind(command.mPass);
            
            Post(command.mTransform);

            Post(light);

            for (auto i = 0; i != command.mMeshNum; ++i)
            {
                Draw(command.mPass->mDrawType, command.mMeshs[i]);
            }
		}
	}
    _renderTarget.Ended();
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

	//	后期处理
    _renderInfo.mPass = nullptr;
}

void Render::RenderForward()
{
    //  打包光源数据
    InitUBOLightForward();
    PackUBOLightForward();
    for (auto & commands : _forwardQueues)
    {
        RenderForwardCommands(commands);
    }
}

void Render::RenderDeferred()
{
    InitGBuffer();

    _renderTarget.Start(RenderTarget::BindType::kALL);
    _renderTarget.BindAttachment(RenderTarget::AttachmentType::kCOLOR0, RenderTarget::TextureType::k2D, _gbuffer.mPositionTexture);
    _renderTarget.BindAttachment(RenderTarget::AttachmentType::kCOLOR1, RenderTarget::TextureType::k2D, _gbuffer.mSpecularTexture);
    _renderTarget.BindAttachment(RenderTarget::AttachmentType::kCOLOR2, RenderTarget::TextureType::k2D, _gbuffer.mDiffuseTexture);
    _renderTarget.BindAttachment(RenderTarget::AttachmentType::kCOLOR3, RenderTarget::TextureType::k2D, _gbuffer.mNormalTexture);
    _renderTarget.BindAttachment(RenderTarget::AttachmentType::kDEPTH, RenderTarget::TextureType::k2D, _gbuffer.mDepthTexture);

    uint rtbinds[] = { RenderTarget::AttachmentType::kCOLOR0, RenderTarget::AttachmentType::kCOLOR1, 
                       RenderTarget::AttachmentType::kCOLOR2, RenderTarget::AttachmentType::kCOLOR3 };
    glDrawBuffers(4, rtbinds);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto & commands : _deferredQueues)
    {
        RenderDeferredCommands(commands);
    }

    _renderTarget.Ended();

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
        Shader::SetTexture2D(_renderInfo.mPass->GLID, UNIFORM_GBUFFER_POSIITON, _gbuffer.mPositionTexture, _renderInfo.mTexBase++);
        Shader::SetTexture2D(_renderInfo.mPass->GLID, UNIFORM_GBUFFER_SPECULAR, _gbuffer.mSpecularTexture, _renderInfo.mTexBase++);
        Shader::SetTexture2D(_renderInfo.mPass->GLID, UNIFORM_GBUFFER_DIFFUSE, _gbuffer.mDiffuseTexture, _renderInfo.mTexBase++);
        Shader::SetTexture2D(_renderInfo.mPass->GLID, UNIFORM_GBUFFER_NORMAL, _gbuffer.mNormalTexture, _renderInfo.mTexBase++);
    }

    if (isRenderShadow)
    {
        switch (command.mLight->GetType())
        {
        case Light::Type::kDIRECT: Shader::SetTexture2D(_renderInfo.mPass->GLID, SFormat(UNIFORM_SHADOW_MAP_DIRECT_, 0).c_str(), command.mLight->GetSMP(), _renderInfo.mTexBase); break;
        case Light::Type::kPOINT: Shader::SetTexture3D(_renderInfo.mPass->GLID, SFormat(UNIFORM_SHADOW_MAP_POINT_, 0).c_str(), command.mLight->GetSMP(), _renderInfo.mTexBase); break;
        case Light::Type::kSPOT: Shader::SetTexture2D(_renderInfo.mPass->GLID, SFormat(UNIFORM_SHADOW_MAP_SPOT_, 0).c_str(), command.mLight->GetSMP(), _renderInfo.mTexBase); break;
        }
    }

    Post(command.mLight);
    
    Post(command.mTransform);

    Draw(pass.mDrawType, *command.mMesh);
}

void Render::InitUBOLightForward()
{
    if (_uboLightForward[UBOLightForwardTypeEnum::kDIRECT] == 0 ||
        _uboLightForward[UBOLightForwardTypeEnum::kPOINT] == 0 ||
        _uboLightForward[UBOLightForwardTypeEnum::kSPOT] == 0)
    {
        assert(_uboLightForward[UBOLightForwardTypeEnum::kDIRECT] == 0);
        assert(_uboLightForward[UBOLightForwardTypeEnum::kPOINT] == 0);
        assert(_uboLightForward[UBOLightForwardTypeEnum::kSPOT] == 0);

        auto directLen = LightDirect::GetUBOLength() * LIMIT_LIGHT_DIRECT;
        auto pointLen = LightPoint::GetUBOLength() * LIMIT_LIGHT_POINT;
        auto spotLen = LightSpot::GetUBOLength() * LIMIT_LIGHT_SPOT;

        glGenBuffers(3, _uboLightForward);
        glBindBuffer(GL_UNIFORM_BUFFER, _uboLightForward[kDIRECT]);
        glBufferData(GL_UNIFORM_BUFFER, directLen, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, _uboLightForward[kPOINT]);
        glBufferData(GL_UNIFORM_BUFFER, pointLen, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, _uboLightForward[kSPOT]);
        glBufferData(GL_UNIFORM_BUFFER, spotLen, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}

void Render::PackUBOLightForward()
{
    for (auto i = 0u, directBase = 0u; i != _renderInfo.mCountUseLightDirect; ++i)
    {
        auto direct = reinterpret_cast<LightDirect *>(_lightQueues.at(Light::Type::kDIRECT).at(i).mLight);
        glBindBuffer(GL_UNIFORM_BUFFER, _uboLightForward[kDIRECT]);
        directBase = glsl_tool::UBOAddData<decltype(LightDirect::UBOData::mMatrix)>(directBase, direct->mMatrix);
        directBase = glsl_tool::UBOAddData<decltype(LightDirect::UBOData::mNormal)>(directBase, direct->mNormal);
        directBase = glsl_tool::UBOAddData<decltype(LightDirect::UBOData::mAmbient)>(directBase, direct->mAmbient);
        directBase = glsl_tool::UBOAddData<decltype(LightDirect::UBOData::mDiffuse)>(directBase, direct->mDiffuse);
        directBase = glsl_tool::UBOAddData<decltype(LightDirect::UBOData::mSpecular)>(directBase, direct->mSpecular);
        directBase = glsl_tool::UBOAddData<decltype(LightDirect::UBOData::mPosition)>(directBase, direct->mPosition);
        directBase = glsl_tool::UBOOffsetBase<glm::vec4>(directBase);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    for (auto i = 0, pointBase = 0; i != _renderInfo.mCountUseLightPoint; ++i)
    {
        auto point = reinterpret_cast<LightPoint *>(_lightQueues.at(Light::Type::kPOINT).at(i).mLight);
        glBindBuffer(GL_UNIFORM_BUFFER, _uboLightForward[kPOINT]);
        pointBase = glsl_tool::UBOAddData<decltype(LightPoint::UBOData::mFar)>(pointBase, point->mFar);
        pointBase = glsl_tool::UBOAddData<decltype(LightPoint::UBOData::mNear)>(pointBase, point->mNear);
        pointBase = glsl_tool::UBOAddData<decltype(LightPoint::UBOData::mK0)>(pointBase, point->mK0);
        pointBase = glsl_tool::UBOAddData<decltype(LightPoint::UBOData::mK1)>(pointBase, point->mK1);
        pointBase = glsl_tool::UBOAddData<decltype(LightPoint::UBOData::mK2)>(pointBase, point->mK2);
        pointBase = glsl_tool::UBOAddData<decltype(LightPoint::UBOData::mAmbient)>(pointBase, point->mAmbient);
        pointBase = glsl_tool::UBOAddData<decltype(LightPoint::UBOData::mDiffuse)>(pointBase, point->mDiffuse);
        pointBase = glsl_tool::UBOAddData<decltype(LightPoint::UBOData::mSpecular)>(pointBase, point->mSpecular);
        pointBase = glsl_tool::UBOAddData<decltype(LightPoint::UBOData::mPosition)>(pointBase, point->mPosition);
        pointBase = glsl_tool::UBOOffsetBase<glm::vec4>(pointBase);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    for (auto i = 0, spotBase = 0; i != _renderInfo.mCountUseLightSpot; ++i)
    {
        auto spot = reinterpret_cast<LightSpot *>(_lightQueues.at(Light::Type::kSPOT).at(i).mLight);
        glBindBuffer(GL_UNIFORM_BUFFER, _uboLightForward[kSPOT]);
        spotBase = glsl_tool::UBOAddData<decltype(LightSpot::UBOData::mK0)>(spotBase, spot->mK0);
        spotBase = glsl_tool::UBOAddData<decltype(LightSpot::UBOData::mK1)>(spotBase, spot->mK1);
        spotBase = glsl_tool::UBOAddData<decltype(LightSpot::UBOData::mK2)>(spotBase, spot->mK2);
        spotBase = glsl_tool::UBOAddData<decltype(LightSpot::UBOData::mInCone)>(spotBase, spot->mInCone);
        spotBase = glsl_tool::UBOAddData<decltype(LightSpot::UBOData::mOutCone)>(spotBase, spot->mOutCone);
        spotBase = glsl_tool::UBOAddData<decltype(LightSpot::UBOData::mMatrix)>(spotBase, spot->mMatrix);
        spotBase = glsl_tool::UBOAddData<decltype(LightSpot::UBOData::mNormal)>(spotBase, spot->mNormal);
        spotBase = glsl_tool::UBOAddData<decltype(LightSpot::UBOData::mAmbient)>(spotBase, spot->mAmbient);
        spotBase = glsl_tool::UBOAddData<decltype(LightSpot::UBOData::mDiffuse)>(spotBase, spot->mDiffuse);
        spotBase = glsl_tool::UBOAddData<decltype(LightSpot::UBOData::mSpecular)>(spotBase, spot->mSpecular);
        spotBase = glsl_tool::UBOAddData<decltype(LightSpot::UBOData::mPosition)>(spotBase, spot->mPosition);
        spotBase = glsl_tool::UBOOffsetBase<glm::vec4>(spotBase);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
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

void Render::InitGBuffer()
{
    if (_gbuffer.mPositionTexture == 0)
    {
        assert(_gbuffer.mPositionTexture == 0
            && _gbuffer.mSpecularTexture == 0
            && _gbuffer.mDiffuseTexture == 0
            && _gbuffer.mNormalTexture == 0
            && _gbuffer.mDepthTexture == 0);
        
        glGenTextures(5, &_gbuffer.mPositionTexture);

        auto windowW = Global::Ref().RefCfgCache().At("init")->At("window", "w")->ToInt();
        auto windowH = Global::Ref().RefCfgCache().At("init")->At("window", "h")->ToInt();

        glBindTexture(GL_TEXTURE_2D, _gbuffer.mPositionTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, windowW, windowH, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, _gbuffer.mSpecularTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowW, windowH, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, _gbuffer.mDiffuseTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowW, windowH, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, _gbuffer.mNormalTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowW, windowH, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindTexture(GL_TEXTURE_2D, _gbuffer.mDepthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowW, windowH, 
                                    0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Render::FillGBuffer()
{
}

template <class T, int N>
bool Render::IsEmptyQueueArray(const std::array<T, N> & ary)
{
    for (auto & queue : ary)
    {
        if (!queue.empty())
            return false;
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
    auto count = _renderInfo.mTexBase;
	for (auto i = 0; i != material.mDiffuses.size(); ++i)
	{
        Shader::SetUniform(_renderInfo.mPass->GLID, SFormat(UNIFORM_MATERIAL_DIFFUSE, i).c_str(), material.mDiffuses.at(i), count++);
	}
	if (material.mSpecular != nullptr)
	{
        Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATERIAL_SPECULAR, material.mSpecular, count++);
	}
	if (material.mReflect != nullptr)
	{
        Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATERIAL_REFLECT, material.mReflect, count++);
	}
	if (material.mNormal != nullptr)
	{
        Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATERIAL_NORMAL, material.mNormal, count++);
	}
	if (material.mHeight != nullptr)
	{
        Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATERIAL_HEIGHT, material.mHeight, count++);
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
