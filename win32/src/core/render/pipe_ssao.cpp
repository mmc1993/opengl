#include "pipe_ssao.h"
#include "renderer.h"
#include "../cfg/cfg_manager.h"
#include "../raw/raw_manager.h"

void PipeSSAO::OnAdd(Renderer * renderer, PipeState * state)
{
    auto windowW = Global::Ref().RefCfgManager().At("init", "window", "w")->ToInt();
    auto windowH = Global::Ref().RefCfgManager().At("init", "window", "h")->ToInt();

    glGenTextures(2, &state->mSSAO.mOcclusionTexture0);
    glBindTexture(GL_TEXTURE_2D, state->mSSAO.mOcclusionTexture0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, windowW, windowH, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, state->mSSAO.mOcclusionTexture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, windowW, windowH, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    _program = Global::Ref().RefRawManager().LoadRes<GLProgram>(BUILTIN_PROGRAM_SSAO);
    _screen = Global::Ref().RefRawManager().LoadRes<GLMesh>(BUILTIN_MESH_SCREEN_QUAD);
}

void PipeSSAO::OnDel(Renderer * renderer, PipeState * state)
{
    Global::Ref().RefRawManager().FreeRes(BUILTIN_PROGRAM_SSAO);
    Global::Ref().RefRawManager().FreeRes(BUILTIN_MESH_SCREEN_QUAD);
    _program = nullptr;
    _screen  = nullptr;
}

void PipeSSAO::OnUpdate(Renderer * renderer, PipeState * state)
{
    state->mRenderTarget[0].Start(RenderTarget::BindType::kDRAW);
    
    //  äÖÈ¾ÍêÕûÉî¶È
    glDrawBuffer(RenderTarget::AttachmentType::kNONE);

    for (const auto & command : state->mDepthQueue)
    {
        renderer->Bind( command.mMaterial->GetProgram(), command.mSubPass);
        renderer->Post(&command.mTransform);
        renderer->Post(
            (DrawTypeEnum)command.mMaterial->GetProgram()->GetPass(command.mSubPass).mDrawType, 
            (FragTypeEnum)command.mMaterial->GetProgram()->GetPass(command.mSubPass).mFragType,
            command.mMaterial->GetMesh());
    }
    
    glDrawBuffer(RenderTarget::AttachmentType::kCOLOR0);

    //  äÖÈ¾ÕÚ±ÎÍ¼
    state->mRenderTarget[0].BindAttachment(
        RenderTarget::AttachmentType::kCOLOR0,
        RenderTarget::TextureType::k2D  ,
        state->mSSAO.mOcclusionTexture0);
    renderer->Bind(_program,          0);
    renderer->Post((glm::mat4 *)nullptr);
    _program->BindUniformTex2D(
        UNIFORM_SCREEN_DEPTH, state->mPostScreen.mDepthTexture, 0);
    renderer->Post(DrawTypeEnum::kINDEX, FragTypeEnum::kTRIANGLE, _screen);

    //  Ä£ºýÕÚ±ÎÍ¼
    state->mRenderTarget[0].BindAttachment(
        RenderTarget::AttachmentType::kCOLOR0,
        RenderTarget::TextureType::k2D  ,
        state->mSSAO.mOcclusionTexture1);
    renderer->Bind(_program, 1);
    _program->BindUniformTex2D(
        UNIFORM_SCREEN_SAO, state->mSSAO.mOcclusionTexture0, 0);
    renderer->Post(DrawTypeEnum::kINDEX, FragTypeEnum::kTRIANGLE, _screen);

    //  SSAO => OK
    state->mRenderTarget[0].Ended();
}
