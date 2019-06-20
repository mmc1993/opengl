#include "render.h"
#include "../window/window.h"
#include "../component/light.h"
#include "../component/camera.h"
#include "../component/transform.h"
#include "../cfg/cfg_manager.h"

Render::Render()
{ }

Render::~Render()
{
    glDeleteTextures(LIMIT_LIGHT_DIRECT, _bufferSet.mShadowMap.mDirectTexture);
    glDeleteTextures(LIMIT_LIGHT_POINT, _bufferSet.mShadowMap.mPointTexture);
    glDeleteTextures(LIMIT_LIGHT_SPOT, _bufferSet.mShadowMap.mSpotTexture);

    glDeleteTextures(1, &_bufferSet.mGBuffer.mNormalTexture);
    glDeleteTextures(1, &_bufferSet.mGBuffer.mDiffuseTexture);
    glDeleteTextures(1, &_bufferSet.mGBuffer.mPositionTexture);
    glDeleteTextures(1, &_bufferSet.mGBuffer.mSpecularTexture);
    glDeleteRenderbuffers(1, &_bufferSet.mGBuffer.mDepthBuffer);

    glDeleteTextures(1, &_bufferSet.mOffScreen.mColorTexture);
    glDeleteTextures(1, &_bufferSet.mOffScreen.mDepthTexture);

    glDeleteBuffers(3, _bufferSet.mLightUBO);
}

MatrixStack & Render::GetMatrixStack()
{
    return _matrixStack;
}

void Render::Once()
{
    InitRender();

    std::sort(_cameraQueue.begin(), _cameraQueue.end(), [](const auto & a, const auto & b)
        {
            return a.mOrder < b.mOrder;
        });

    for (const auto & command : _cameraQueue)
    {
        Bind(&command);
        RenderCamera();
        Bind((CameraCommand *)nullptr);
    }
	ClearCommands();
}

void Render::Post(const RenderCommand::TypeEnum type, const RenderCommand & command)
{
    switch (type)
    {
    case RenderCommand::kMATERIAL:
        {
            auto cmd = (MaterialCommand &) command;
            auto pro = cmd.mMaterial->GetProgram();
            for (auto i = 0; i != pro->GetPasss().size(); ++i)
            {
                cmd.mSubPass = i;
                auto &  pass = cmd.mMaterial->GetProgram()->GetPass(i);
                switch (pass.mRenderType)
                {
                case RenderTypeEnum::kSHADOW: _shadowQueue.push_back(cmd); break;
                case RenderTypeEnum::kFORWARD: _forwardQueues.at(pass.mRenderQueue).push_back(cmd); break;
                case RenderTypeEnum::kDEFERRED: _deferredQueues.at(pass.mRenderQueue).push_back(cmd); break;
                }
            }
        }
        break;
    case RenderCommand::kCAMERA:
        {
            _cameraQueue.push_back((const CameraCommand &)command);
        }
        break;
    case RenderCommand::kLIGHT:
        {
            auto & cmd = (const LightCommand &)command;
            _lightQueues.at(cmd.mLight->GetType()).push_back(cmd);
        }
        break;
    }
}

void Render::BakeLightDepthMap(Light * light, uint shadow)
{
    _renderState.mProgram = nullptr;
    _renderTarget[0].Start();
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    for (auto i = 0; light->NextDrawShadow(i, shadow, &_renderTarget[0]); ++i)
	{
        glClear(GL_DEPTH_BUFFER_BIT);
		for (auto & command : _shadowQueue)
		{
            if (Bind(command.mMaterial->GetProgram(), command.mSubPass))
            {
                Post(light);
            }
            Post(command.mTransform);
            Post((DrawTypeEnum)command.mMaterial->GetProgram()->GetPass(command.mSubPass).mDrawType, command.mMaterial->GetMesh());
        }
	}
    _renderTarget[0].Ended();
}

void Render::SortLightCommands()
{
    const auto & cameraPos = _renderState.mCamera->mPos;

    std::sort(_lightQueues.at(Light::kPOINT).begin(), 
              _lightQueues.at(Light::kPOINT).end(),
              [&cameraPos](const LightCommand & command0, const LightCommand & command1)
        {
            auto diff0 = (cameraPos - command0.mLight->mPosition);
            auto diff1 = (cameraPos - command1.mLight->mPosition);
            return glm::dot(diff0, diff0) < glm::dot(diff1, diff1);
        });

    std::sort(_lightQueues.at(Light::kSPOT).begin(), 
              _lightQueues.at(Light::kSPOT).end(),
              [&cameraPos](const LightCommand & command0, const LightCommand & command1)
        {
            auto diff0 = (cameraPos - command0.mLight->mPosition);
            auto diff1 = (cameraPos - command1.mLight->mPosition);
            return glm::dot(diff0, diff0) < glm::dot(diff1, diff1);
        });
}

void Render::BakeLightDepthMap()
{
    for (auto i = 0; i != std::min(_lightQueues.at(Light::kDIRECT).size(), LIMIT_LIGHT_DIRECT); ++i)
    {
        BakeLightDepthMap(_lightQueues.at(Light::kDIRECT).at(i).mLight, _bufferSet.mShadowMap.mDirectTexture[i]);
    }
    for (auto i = 0; i != std::min(_lightQueues.at(Light::kPOINT).size(), LIMIT_LIGHT_POINT); ++i)
    {
        BakeLightDepthMap(_lightQueues.at(Light::kPOINT).at(i).mLight, _bufferSet.mShadowMap.mPointTexture[i]);
    }
    for (auto i = 0; i != std::min(_lightQueues.at(Light::kSPOT).size(), LIMIT_LIGHT_SPOT); ++i)
    {
        BakeLightDepthMap(_lightQueues.at(Light::kSPOT).at(i).mLight, _bufferSet.mShadowMap.mSpotTexture[i]);
    }
    ASSERT_LOG(_renderState.mCamera != nullptr, "_renderState.mCamera != nullptr");
    glViewport(
        (iint)_renderState.mCamera->mViewport.x,
        (iint)_renderState.mCamera->mViewport.y,
        (iint)_renderState.mCamera->mViewport.z,
        (iint)_renderState.mCamera->mViewport.w);
}

void Render::RenderCamera()
{
    SortLightCommands();

    BakeLightDepthMap();

    //  延迟渲染
    _renderState.mProgram = nullptr;
    RenderDeferred();
    
    //  正向渲染
    _renderState.mProgram = nullptr;
    RenderForward();
    
    _renderTarget[1].Start(RenderTarget::BindType::kREAD);
    glBlitFramebuffer(
        0, 0,
        Global::Ref().RefWindow().GetW(),
        Global::Ref().RefWindow().GetH(),
        0, 0,
        Global::Ref().RefWindow().GetW(),
        Global::Ref().RefWindow().GetH(),
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    _renderTarget[1].Ended();

	//	后期处理
    _renderState.mProgram = nullptr;
}

void Render::RenderForward()
{
    PackUBOLightForward();

    _renderTarget[1].Start();
    for (auto & commands : _forwardQueues)
    {
        for (const auto & command : commands)
        {
            if ((_renderState.mCamera->mMask & command.mCameraMask) != 0)
            {
                if (Bind(command.mMaterial->GetProgram(), command.mSubPass))
                {
                    BindUBOLightForward();
                }
                Post(command.mMaterial);
                Post(command.mTransform);
                Post((DrawTypeEnum)command.mMaterial->GetProgram()->GetPass(command.mSubPass).mDrawType, command.mMaterial->GetMesh());
            }
        }
    }
    _renderTarget[1].Ended();
}

void Render::RenderDeferred()
{
    _renderTarget[0].Start(RenderTarget::BindType::kALL);
    _renderTarget[0].BindAttachment(RenderTarget::AttachmentType::kCOLOR0, RenderTarget::TextureType::k2D, _bufferSet.mGBuffer.mPositionTexture);
    _renderTarget[0].BindAttachment(RenderTarget::AttachmentType::kCOLOR1, RenderTarget::TextureType::k2D, _bufferSet.mGBuffer.mSpecularTexture);
    _renderTarget[0].BindAttachment(RenderTarget::AttachmentType::kCOLOR2, RenderTarget::TextureType::k2D, _bufferSet.mGBuffer.mDiffuseTexture);
    _renderTarget[0].BindAttachment(RenderTarget::AttachmentType::kCOLOR3, RenderTarget::TextureType::k2D, _bufferSet.mGBuffer.mNormalTexture);
    _renderTarget[0].BindAttachment(RenderTarget::AttachmentType::kDEPTH, _bufferSet.mGBuffer.mDepthBuffer);

    uint outputs[] = { RenderTarget::AttachmentType::kCOLOR0, RenderTarget::AttachmentType::kCOLOR1, 
                       RenderTarget::AttachmentType::kCOLOR2, RenderTarget::AttachmentType::kCOLOR3 };
    glDrawBuffers(4, outputs);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto & commands : _deferredQueues)
    {
        for (auto & command : commands)
        {
            if ((_renderState.mCamera->mMask & command.mCameraMask) != 0)
            {
                Bind(command.mMaterial->GetProgram(), command.mSubPass);
                Post(command.mMaterial);
                Post(command.mTransform);
                Post((DrawTypeEnum)command.mMaterial->GetProgram()->GetPass(command.mSubPass).mDrawType, command.mMaterial->GetMesh());
            }
        }
    }

    _renderTarget[0].Start(RenderTarget::BindType::kREAD);
    _renderTarget[1].Start(RenderTarget::BindType::kDRAW);
    glBlitFramebuffer(
        0, 0,
        Global::Ref().RefWindow().GetW(),
        Global::Ref().RefWindow().GetH(),
        0, 0,
        Global::Ref().RefWindow().GetW(),
        Global::Ref().RefWindow().GetH(),
        GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    _renderTarget[0].Ended();

    for (auto i = 0u; i != _lightQueues.at(Light::kDIRECT).size(); ++i)
    {
        RenderDeferredLightVolume(_lightQueues.at(Light::kDIRECT).at(i), i < LIMIT_LIGHT_DIRECT? _bufferSet.mShadowMap.mDirectTexture[i]: 0);
    }
    for (auto i = 0u; i != _lightQueues.at(Light::kPOINT).size(); ++i)
    {
        RenderDeferredLightVolume(_lightQueues.at(Light::kPOINT).at(i), i < LIMIT_LIGHT_POINT ? _bufferSet.mShadowMap.mPointTexture[i] : 0);
    }
    for (auto i = 0u; i != _lightQueues.at(Light::kSPOT).size(); ++i)
    {
        RenderDeferredLightVolume(_lightQueues.at(Light::kSPOT).at(i), i < LIMIT_LIGHT_SPOT ? _bufferSet.mShadowMap.mSpotTexture[i] : 0);
    }
    _renderTarget[1].Ended();
}

void Render::RenderDeferredLightVolume(const LightCommand & command, uint shadow)
{
    if (Bind(command.mProgram, shadow != 0u? 0u: 1u))
    {
        _renderState.mProgram->BindUniformTex2D(UNIFORM_GBUFFER_POSIITON, _bufferSet.mGBuffer.mPositionTexture, _renderState.mTexBase + 0);
        _renderState.mProgram->BindUniformTex2D(UNIFORM_GBUFFER_SPECULAR, _bufferSet.mGBuffer.mSpecularTexture, _renderState.mTexBase + 1);
        _renderState.mProgram->BindUniformTex2D(UNIFORM_GBUFFER_DIFFUSE, _bufferSet.mGBuffer.mDiffuseTexture, _renderState.mTexBase + 2);
        _renderState.mProgram->BindUniformTex2D(UNIFORM_GBUFFER_NORMAL, _bufferSet.mGBuffer.mNormalTexture, _renderState.mTexBase + 3);
    }
    ASSERT_LOG(command.mProgram->GetPass(0).mRenderType == RenderTypeEnum::kLIGHT, "command.mProgram->GetPass(0).vRenderType == RenderTypeEnum::kLIGHT. {0}", command.mProgram->GetPass(0).mRenderType);
    ASSERT_LOG(command.mProgram->GetPass(1).mRenderType == RenderTypeEnum::kLIGHT, "command.mProgram->GetPass(1).vRenderType == RenderTypeEnum::kLIGHT. {0}", command.mProgram->GetPass(1).mRenderType);
    if (shadow != 0)
    {
        switch (command.mLight->GetType())
        {
        case Light::kDIRECT: _renderState.mProgram->BindUniformTex2D(SFormat(UNIFORM_SHADOW_MAP_DIRECT_, 0).c_str(), shadow, _renderState.mTexBase + 4); break;
        case Light::kPOINT: _renderState.mProgram->BindUniformTex3D(SFormat(UNIFORM_SHADOW_MAP_POINT_, 0).c_str(), shadow, _renderState.mTexBase + 4); break;
        case Light::kSPOT: _renderState.mProgram->BindUniformTex2D(SFormat(UNIFORM_SHADOW_MAP_SPOT_, 0).c_str(), shadow, _renderState.mTexBase + 4); break;
        }
    }
    Post(command.mLight);
    Post(command.mTransform);
    Post(DrawTypeEnum::kINDEX, command.mMesh);
}

void Render::PackUBOLightForward()
{
    const auto DIRECT_UBO_LEN = LightDirect::GetUBOLength();
    const auto POINT_UBO_LEN = LightPoint::GetUBOLength();
    const auto SPOT_UBO_LEN = LightSpot::GetUBOLength();

    auto offset = 0;
    glBindBuffer(GL_COPY_WRITE_BUFFER, _bufferSet.mLightUBO[Light::kDIRECT]);
    for (auto i = 0u; i != std::min(_lightQueues.at(Light::kDIRECT).size(), LIMIT_LIGHT_DIRECT); ++i)
    {
        glBindBuffer(GL_COPY_READ_BUFFER, _lightQueues.at(Light::kDIRECT).at(i).mLight->GetUBO());
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, offset, DIRECT_UBO_LEN);
        glBindBuffer(GL_COPY_READ_BUFFER, 0);
        offset += DIRECT_UBO_LEN;
    }

    offset = 0;
    glBindBuffer(GL_COPY_WRITE_BUFFER, _bufferSet.mLightUBO[Light::kPOINT]);
    for (auto i = 0; i != std::min(_lightQueues.at(Light::kPOINT).size(), LIMIT_LIGHT_POINT); ++i)
    {
        glBindBuffer(GL_COPY_READ_BUFFER, _lightQueues.at(Light::kPOINT).at(i).mLight->GetUBO());
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, offset, POINT_UBO_LEN);
        glBindBuffer(GL_COPY_READ_BUFFER, 0);
        offset += POINT_UBO_LEN;
    }

    offset = 0;
    glBindBuffer(GL_COPY_WRITE_BUFFER, _bufferSet.mLightUBO[Light::kSPOT]);
    for (auto i = 0; i != std::min(_lightQueues.at(Light::kSPOT).size(), LIMIT_LIGHT_SPOT); ++i)
    {
        glBindBuffer(GL_COPY_READ_BUFFER, _lightQueues.at(Light::kSPOT).at(i).mLight->GetUBO());
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, offset, SPOT_UBO_LEN);
        glBindBuffer(GL_COPY_READ_BUFFER, 0);
        offset += SPOT_UBO_LEN;
    }

    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}

void Render::BindUBOLightForward()
{
    auto countDirect = std::min(_lightQueues.at(Light::kDIRECT).size(), LIMIT_LIGHT_DIRECT);
    auto countPoint = std::min(_lightQueues.at(Light::kPOINT).size(), LIMIT_LIGHT_POINT);
    auto countSpot = std::min(_lightQueues.at(Light::kSPOT).size(), LIMIT_LIGHT_SPOT);

    auto indexDirect = glGetUniformBlockIndex(_renderState.mProgram->GetUseID(), UBO_NAME_LIGHT_DIRECT);
    auto indexPoint = glGetUniformBlockIndex(_renderState.mProgram->GetUseID(), UBO_NAME_LIGHT_POINT);
    auto indexSpot = glGetUniformBlockIndex(_renderState.mProgram->GetUseID(), UBO_NAME_LIGHT_SPOT);

    glUniformBlockBinding(_renderState.mProgram->GetUseID(), indexDirect, UniformBlockEnum::kLIGHT_DIRECT);
    glUniformBlockBinding(_renderState.mProgram->GetUseID(), indexPoint, UniformBlockEnum::kLIGHT_POINT);
    glUniformBlockBinding(_renderState.mProgram->GetUseID(), indexSpot, UniformBlockEnum::kLIGHT_SPOT);

    glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_DIRECT, _bufferSet.mLightUBO[Light::kDIRECT]);
    glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_POINT, _bufferSet.mLightUBO[Light::kPOINT]);
    glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_SPOT, _bufferSet.mLightUBO[Light::kSPOT]);

    _renderState.mProgram->BindUniformNumber(UNIFORM_LIGHT_COUNT_DIRECT_, countDirect);
    _renderState.mProgram->BindUniformNumber(UNIFORM_LIGHT_COUNT_POINT_, countPoint);
    _renderState.mProgram->BindUniformNumber(UNIFORM_LIGHT_COUNT_SPOT_, countSpot);

    for (auto i = 0, directCount = 0; i != countDirect; ++i, ++directCount)
    {
        _renderState.mProgram->BindUniformTex2D(SFormat(UNIFORM_SHADOW_MAP_DIRECT_, directCount).c_str(), _bufferSet.mShadowMap.mDirectTexture[i], _renderState.mTexBase++);
    }
    for (auto i = 0, pointCount = 0; i != countPoint; ++i, ++pointCount)
    {
        _renderState.mProgram->BindUniformTex3D(SFormat(UNIFORM_SHADOW_MAP_POINT_, pointCount).c_str(), _bufferSet.mShadowMap.mPointTexture[i], _renderState.mTexBase++);
    }
    for (auto i = 0, spotCount = 0; i != countSpot; ++i, ++spotCount)
    {
        _renderState.mProgram->BindUniformTex2D(SFormat(UNIFORM_SHADOW_MAP_SPOT_, spotCount).c_str(), _bufferSet.mShadowMap.mSpotTexture[i], _renderState.mTexBase++);
    }
}

void Render::Bind(const CameraCommand * command)
{
	if (command != nullptr)
	{
		Global::Ref().RefRender().GetMatrixStack().Identity(MatrixStack::kVIEW);
		Global::Ref().RefRender().GetMatrixStack().Identity(MatrixStack::kPROJ);
		Global::Ref().RefRender().GetMatrixStack().Mul(MatrixStack::kVIEW, command->mView);
		Global::Ref().RefRender().GetMatrixStack().Mul(MatrixStack::kPROJ, command->mProj);
		glViewport((int)command->mViewport.x, (int)command->mViewport.y,
				   (int)command->mViewport.z, (int)command->mViewport.w);
        _renderState.mCamera = command;
	}
	else
	{
		Global::Ref().RefRender().GetMatrixStack().Pop(MatrixStack::kVIEW);
		Global::Ref().RefRender().GetMatrixStack().Pop(MatrixStack::kPROJ);
        _renderState.mCamera = nullptr;
	}
}

void Render::Post(const Light * light)
{
    switch (light->GetType())
    {
    case Light::kDIRECT:
        {
            auto idx = glGetUniformBlockIndex(_renderState.mProgram->GetUseID(), UBO_NAME_LIGHT_DIRECT);
            if (GL_INVALID_INDEX != idx)
            {
                glUniformBlockBinding(_renderState.mProgram->GetUseID(), idx, UniformBlockEnum::kLIGHT_DIRECT);
                glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_DIRECT, light->GetUBO());
            }
        }
        break;
    case Light::kPOINT:
        {
            auto idx = glGetUniformBlockIndex(_renderState.mProgram->GetUseID(), UBO_NAME_LIGHT_POINT);
            if (GL_INVALID_INDEX != idx)
            {
                glUniformBlockBinding(_renderState.mProgram->GetUseID(), idx, UniformBlockEnum::kLIGHT_POINT);
                glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_POINT, light->GetUBO());
            }
        }
        break;
    case Light::kSPOT:
        {
            auto idx = glGetUniformBlockIndex(_renderState.mProgram->GetUseID(), UBO_NAME_LIGHT_SPOT);
            if (GL_INVALID_INDEX != idx)
            {
                glUniformBlockBinding(_renderState.mProgram->GetUseID(), idx, UniformBlockEnum::kLIGHT_SPOT);
                glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_SPOT, light->GetUBO());
            }
        }
        break;
    }
    _renderState.mProgram->BindUniformNumber(UNIFORM_LIGHT_TYPE, light->GetType());
}

void Render::InitRender()
{
    if (_bufferSet.mGBuffer.mPositionTexture == 0)
    {
        auto shadowW = Global::Ref().RefCfgManager().At("init", "shadow_map", "w")->ToInt();
        auto shadowH = Global::Ref().RefCfgManager().At("init", "shadow_map", "h")->ToInt();
        auto windowW = Global::Ref().RefCfgManager().At("init")->At("window", "w")->ToInt();
        auto windowH = Global::Ref().RefCfgManager().At("init")->At("window", "h")->ToInt();

        //  光源UBO
        glGenBuffers(3, _bufferSet.mLightUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, _bufferSet.mLightUBO[Light::kDIRECT]);
        glBufferData(GL_UNIFORM_BUFFER, LightDirect::GetUBOLength() * LIMIT_LIGHT_DIRECT, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, _bufferSet.mLightUBO[Light::kPOINT]);
        glBufferData(GL_UNIFORM_BUFFER, LightPoint::GetUBOLength() * LIMIT_LIGHT_POINT, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, _bufferSet.mLightUBO[Light::kSPOT]);
        glBufferData(GL_UNIFORM_BUFFER, LightSpot::GetUBOLength() * LIMIT_LIGHT_SPOT, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        //  阴影贴图
        glGenTextures(LIMIT_LIGHT_DIRECT, _bufferSet.mShadowMap.mDirectTexture);
        glGenTextures(LIMIT_LIGHT_POINT, _bufferSet.mShadowMap.mPointTexture);
        glGenTextures(LIMIT_LIGHT_SPOT, _bufferSet.mShadowMap.mSpotTexture);
        for (auto i = 0; i != LIMIT_LIGHT_DIRECT; ++i)
        {
            glBindTexture(GL_TEXTURE_2D, _bufferSet.mShadowMap.mDirectTexture[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowW, shadowH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        for (auto i = 0; i != LIMIT_LIGHT_POINT; ++i)
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, _bufferSet.mShadowMap.mPointTexture[i]);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_DEPTH_COMPONENT, shadowW, shadowH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_DEPTH_COMPONENT, shadowW, shadowH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_DEPTH_COMPONENT, shadowW, shadowH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_DEPTH_COMPONENT, shadowW, shadowH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_DEPTH_COMPONENT, shadowW, shadowH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_DEPTH_COMPONENT, shadowW, shadowH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        }
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        for (auto i = 0; i != LIMIT_LIGHT_SPOT; ++i)
        {
            glBindTexture(GL_TEXTURE_2D, _bufferSet.mShadowMap.mSpotTexture[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowW, shadowH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        //  G-Buffer
        glGenTextures(4,  &_bufferSet.mGBuffer.mPositionTexture);
        glGenRenderbuffers(1, &_bufferSet.mGBuffer.mDepthBuffer);

        glBindTexture(GL_TEXTURE_2D, _bufferSet.mGBuffer.mPositionTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowW, windowH, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, _bufferSet.mGBuffer.mSpecularTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowW, windowH, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, _bufferSet.mGBuffer.mDiffuseTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowW, windowH, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, _bufferSet.mGBuffer.mNormalTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowW, windowH, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindRenderbuffer(GL_RENDERBUFFER, _bufferSet.mGBuffer.mDepthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowW, windowH);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        //  Off Screen Texture
        glGenTextures(2, &_bufferSet.mOffScreen.mColorTexture);

        glBindTexture(GL_TEXTURE_2D, _bufferSet.mOffScreen.mColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowW, windowH, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, _bufferSet.mOffScreen.mDepthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowW, windowH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        _renderTarget[1].Start();
        _renderTarget[1].BindAttachment(RenderTarget::AttachmentType::kCOLOR0, RenderTarget::TextureType::k2D, _bufferSet.mOffScreen.mColorTexture);
        _renderTarget[1].BindAttachment(RenderTarget::AttachmentType::kDEPTH,  RenderTarget::TextureType::k2D, _bufferSet.mOffScreen.mDepthTexture);
        _renderTarget[1].Ended();
    }

    _renderState.mVertexCount = 0;
    _renderState.mRenderCount = 0;

    _renderTarget[1].Start();
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);
    _renderTarget[1].Ended();
}

bool Render::Bind(const GLProgram * program, uint pass)
{
    if (_renderState.mProgram != program)
    {
        _renderState.mTexBase = 0;
        _renderState.mProgram = program;
        _renderState.mProgram->UsePass(pass, true);
        return true;
    }
    return _renderState.mProgram->UsePass(pass);
}

void Render::Post(const GLMaterial * material)
{
    for (auto i = 0; i != material->GetItems().size(); ++i)
    {
        auto & item = material->GetItems().at(i);
        auto key    = SFormat(UNIFORM_MATERIAL, item.mKey);
        switch (item.mType)
        {
        case GLMaterial::Item::kNUMBER: { _renderState.mProgram->BindUniformNumber(key.c_str(), std::any_cast<const float &>(item.mVal)); } break;
        case GLMaterial::Item::kTEX2D: { _renderState.mProgram->BindUniformTex2D(key.c_str(), std::any_cast<GLTexture2D *>(item.mVal)->GetID(), _renderState.mTexBase + i); } break;
        case GLMaterial::Item::kTEX3D: { _renderState.mProgram->BindUniformTex3D(key.c_str(), std::any_cast<GLTexture2D *>(item.mVal)->GetID(), _renderState.mTexBase + i); } break;
        }
    }
}

void Render::ClearCommands()
{
    _cameraQueue.clear();
    _shadowQueue.clear();
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
    _renderState.mProgram->BindUniformMatrix(UNIFORM_MATRIX_N, matrixN);
    _renderState.mProgram->BindUniformMatrix(UNIFORM_MATRIX_M, matrixM);
    _renderState.mProgram->BindUniformMatrix(UNIFORM_MATRIX_V, matrixV);
    _renderState.mProgram->BindUniformMatrix(UNIFORM_MATRIX_P, matrixP);
    _renderState.mProgram->BindUniformMatrix(UNIFORM_MATRIX_MV,  matrixMV);
    _renderState.mProgram->BindUniformMatrix(UNIFORM_MATRIX_MVP, matrixMVP);
    _renderState.mProgram->BindUniformNumber(UNIFORM_GAME_TIME, glfwGetTime());
    if (_renderState.mCamera != nullptr)
    {
        _renderState.mProgram->BindUniformVector(UNIFORM_CAMERA_POS, _renderState.mCamera->mPos);
        _renderState.mProgram->BindUniformVector(UNIFORM_CAMERA_EYE, _renderState.mCamera->mEye);
    }
}

void Render::Post(DrawTypeEnum drawType, const GLMesh * mesh)
{
    ASSERT_LOG(mesh->GetVAO() != 0, "Draw VAO Error");

	glBindVertexArray(mesh->GetVAO());
	switch (drawType)
	{
	case DrawTypeEnum::kINSTANCE:
		{
			//	TODO, 暂不实现
		}
		break;
	case DrawTypeEnum::kVERTEX:
		{
			_renderState.mVertexCount += mesh->GetVCount();
			glDrawArrays(GL_TRIANGLES, 0, mesh->GetVCount());
		}
		break;
	case DrawTypeEnum::kINDEX:
		{
            _renderState.mVertexCount += mesh->GetECount();
			glDrawElements(GL_TRIANGLES, mesh->GetECount(), GL_UNSIGNED_INT, nullptr);
		}
		break;
	}
	++_renderState.mRenderCount;
}
