#include "pipe_gbuffer.h"
#include "renderer.h"
#include "../cfg/cfg_manager.h"

void PipeGBuffer::OnAdd(Renderer * renderer, PipeState * state)
{
    auto windowW = Global::Ref().RefCfgManager().At("init", "window", "w")->ToInt();
    auto windowH = Global::Ref().RefCfgManager().At("init", "window", "h")->ToInt();

    glGenTextures(3, &state->mGBuffer.mPositionTexture);

    glBindTexture(GL_TEXTURE_2D, state->mGBuffer.mPositionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowW, windowH, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, state->mGBuffer.mDiffuseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowW, windowH, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, state->mGBuffer.mNormalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowW, windowH, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void PipeGBuffer::OnDel(Renderer * renderer, PipeState * state)
{
}

void PipeGBuffer::OnUpdate(Renderer * renderer, PipeState * state)
{
    state->mRenderTarget[0].Start(RenderTarget::BindType::kALL);
    state->mRenderTarget[0].BindAttachment(RenderTarget::AttachmentType::kCOLOR0, RenderTarget::TextureType::k2D, state->mGBuffer.mPositionTexture);
    state->mRenderTarget[0].BindAttachment(RenderTarget::AttachmentType::kCOLOR1, RenderTarget::TextureType::k2D, state->mGBuffer.mDiffuseTexture);
    state->mRenderTarget[0].BindAttachment(RenderTarget::AttachmentType::kCOLOR2, RenderTarget::TextureType::k2D, state->mGBuffer.mNormalTexture);
    state->mRenderTarget[0].BindAttachment(RenderTarget::AttachmentType::kDEPTH, RenderTarget::TextureType::k2D, state->mPostScreen.mDepthTexture);

    uint outputs[] = {
        RenderTarget::AttachmentType::kCOLOR0,
        RenderTarget::AttachmentType::kCOLOR1,
        RenderTarget::AttachmentType::kCOLOR2 };
    glDrawBuffers(std::length(outputs), outputs);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto & commands : state->mDeferredQueues)
    {
        for (auto & command : commands)
        {
            if ((state->mRenderTime.mCamera->mMask & command.mCameraMask) != 0)
            {
                renderer->Bind(command.mMaterial->GetProgram(), command.mSubPass);
                renderer->Post(command.mMaterial  );
                renderer->Post(&command.mTransform);
                renderer->Post(
                    (DrawTypeEnum)command.mMaterial->GetProgram()->GetPass(command.mSubPass).mDrawType, 
                    (FragTypeEnum)command.mMaterial->GetProgram()->GetPass(command.mSubPass).mFragType,
                    command.mMaterial->GetMesh());
            }
        }
    }
    state->mRenderTarget[0].Ended();
}
