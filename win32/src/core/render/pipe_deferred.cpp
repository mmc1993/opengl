#include "pipe_deferred.h"
#include "renderer.h"
#include "../component/light.h"

void PipeDeferred::OnAdd(Renderer * renderer, PipeState * state)
{
    //  无需实现
}

void PipeDeferred::OnDel(Renderer * renderer, PipeState * state)
{
    //  无需实现
}

void PipeDeferred::OnUpdate(Renderer * renderer, PipeState * state)
{
    state->mRenderTarget[1].Start(RenderTarget::BindType::kDRAW);
    for (auto i = 0u; i != state->mLightQueues.at(Light::kDIRECT).size(); ++i)
    {
        RenderLightVolume(renderer, state, &state->mLightQueues.at(Light::kDIRECT).at(i), i < LIMIT_LIGHT_DIRECT ? state->mShadowMap.mDirectTexture[i] : 0);
    }
    for (auto i = 0u; i != state->mLightQueues.at(Light::kPOINT).size(); ++i)
    {
        RenderLightVolume(renderer, state, &state->mLightQueues.at(Light::kPOINT).at(i), i < LIMIT_LIGHT_POINT ? state->mShadowMap.mPointTexture[i] : 0);
    }
    for (auto i = 0u; i != state->mLightQueues.at(Light::kSPOT).size(); ++i)
    {
        RenderLightVolume(renderer, state, &state->mLightQueues.at(Light::kSPOT).at(i), i < LIMIT_LIGHT_SPOT ? state->mShadowMap.mSpotTexture[i] : 0);
    }
    state->mRenderTarget[1].Ended();
}

void PipeDeferred::RenderLightVolume(Renderer * renderer, PipeState * state, const LightCommand * command, uint shadow)
{
    if (renderer->Bind(command->mProgram, shadow != 0u ? 0u : 1u))
    {
        state->mRenderTime.mProgram->BindUniformTex2D(UNIFORM_GBUFFER_POSIITON, state->mGBuffer.mPositionTexture, state->mRenderTime.mTexBase + 0);
        state->mRenderTime.mProgram->BindUniformTex2D(UNIFORM_GBUFFER_DIFFUSE, state->mGBuffer.mDiffuseTexture, state->mRenderTime.mTexBase + 1);
        state->mRenderTime.mProgram->BindUniformTex2D(UNIFORM_GBUFFER_NORMAL, state->mGBuffer.mNormalTexture, state->mRenderTime.mTexBase + 2);
        state->mRenderTime.mProgram->BindUniformTex2D(UNIFORM_SCREEN_SAO, state->mSSAO.mOcclusionTexture1, state->mRenderTime.mTexBase + 3);
    }
    ASSERT_LOG(command->mProgram->GetPass(0).mRenderType == RenderTypeEnum::kLIGHT, "command.mProgram->GetPass(0).vRenderType == RenderTypeEnum::kLIGHT. {0}", command->mProgram->GetPass(0).mRenderType);
    ASSERT_LOG(command->mProgram->GetPass(1).mRenderType == RenderTypeEnum::kLIGHT, "command.mProgram->GetPass(1).vRenderType == RenderTypeEnum::kLIGHT. {0}", command->mProgram->GetPass(1).mRenderType);
    if (shadow != 0)
    {
        switch (command->mType)
        {
        case Light::kDIRECT: state->mRenderTime.mProgram->BindUniformTex2D(SFormat(UNIFORM_SHADOW_MAP_DIRECT_, 0).c_str(), shadow, state->mRenderTime.mTexBase + 4); break;
        case Light::kPOINT: state->mRenderTime.mProgram->BindUniformTex3D(SFormat(UNIFORM_SHADOW_MAP_POINT_, 0).c_str(), shadow, state->mRenderTime.mTexBase + 4); break;
        case Light::kSPOT: state->mRenderTime.mProgram->BindUniformTex2D(SFormat(UNIFORM_SHADOW_MAP_SPOT_, 0).c_str(), shadow, state->mRenderTime.mTexBase + 4); break;
        }
    }
    renderer->Post( command);
    renderer->Post(&command->mTransform);
    renderer->Post(DrawTypeEnum::kINDEX,
                   FragTypeEnum::kTRIANGLE,
                   command->mMesh);
}