#include "pipe_forward.h"
#include "renderer.h"
#include "../component/light.h"

void PipeForward::OnAdd(Renderer * renderer, PipeState * state)
{
    glGenBuffers(3, state->mLightUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, state->mLightUBO[Light::kDIRECT]);
    glBufferData(GL_UNIFORM_BUFFER, LightDirect::GetUBOLength() * LIMIT_LIGHT_DIRECT, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, state->mLightUBO[Light::kPOINT]);
    glBufferData(GL_UNIFORM_BUFFER, LightPoint::GetUBOLength() * LIMIT_LIGHT_POINT, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, state->mLightUBO[Light::kSPOT]);
    glBufferData(GL_UNIFORM_BUFFER, LightSpot::GetUBOLength() * LIMIT_LIGHT_SPOT, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void PipeForward::OnDel(Renderer * renderer, PipeState * state)
{
}

void PipeForward::OnUpdate(Renderer * renderer, PipeState * state)
{
    PackUBO(renderer, state);

    state->mRenderTarget[1].Start();
    for (auto & commands : state->mForwardQueues)
    {
        for (const auto & command : commands)
        {
            if ((state->mRenderTime.mCamera->mMask & command.mCameraMask) != 0)
            {
                if (renderer->Bind(command.mMaterial->GetProgram(), command.mSubPass))
                {
                    renderer->Post(command.mProgramParam);
                    BindUBO(renderer, state);
                }
                renderer->Post( command.mMaterial );
                renderer->Post(&command.mTransform);
                renderer->Post(
                    (DrawTypeEnum)command.mMaterial->GetProgram()->GetPass(command.mSubPass).mDrawType, 
                    (FragTypeEnum)command.mMaterial->GetProgram()->GetPass(command.mSubPass).mFragType,
                    command.mMaterial->GetMesh());
            }
        }
    }
    state->mRenderTarget[1].Ended();
}

void PipeForward::PackUBO(Renderer * renderer, PipeState * state)
{
    const auto DIRECT_UBO_LEN = LightDirect::GetUBOLength();
    const auto POINT_UBO_LEN = LightPoint::GetUBOLength();
    const auto SPOT_UBO_LEN = LightSpot::GetUBOLength();

    auto offset = 0;
    glBindBuffer(GL_COPY_WRITE_BUFFER, state->mLightUBO[Light::kDIRECT]);
    for (auto i = 0u; i != std::min(state->mLightQueues.at(Light::kDIRECT).size(), LIMIT_LIGHT_DIRECT); ++i)
    {
        glBindBuffer(GL_COPY_READ_BUFFER, state->mLightQueues.at(Light::kDIRECT).at(i).mUBO);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, offset, DIRECT_UBO_LEN);
        offset += DIRECT_UBO_LEN;
    }

    offset = 0;
    glBindBuffer(GL_COPY_WRITE_BUFFER, state->mLightUBO[Light::kPOINT]);
    for (auto i = 0; i != std::min(state->mLightQueues.at(Light::kPOINT).size(), LIMIT_LIGHT_POINT); ++i)
    {
        glBindBuffer(GL_COPY_READ_BUFFER, state->mLightQueues.at(Light::kPOINT).at(i).mUBO);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, offset, POINT_UBO_LEN);
        offset += POINT_UBO_LEN;
    }

    offset = 0;
    glBindBuffer(GL_COPY_WRITE_BUFFER, state->mLightUBO[Light::kSPOT]);
    for (auto i = 0; i != std::min(state->mLightQueues.at(Light::kSPOT).size(), LIMIT_LIGHT_SPOT); ++i)
    {
        glBindBuffer(GL_COPY_READ_BUFFER, state->mLightQueues.at(Light::kSPOT).at(i).mUBO);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, offset, SPOT_UBO_LEN);
        offset += SPOT_UBO_LEN;
    }

    glBindBuffer(GL_COPY_READ_BUFFER,  0);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}

void PipeForward::BindUBO(Renderer * renderer, PipeState * state)
{
    auto index0 = glGetUniformBlockIndex(state->mRenderTime.mProgram->GetUseID(), UBO_NAME_LIGHT_DIRECT);
    auto index1 = glGetUniformBlockIndex(state->mRenderTime.mProgram->GetUseID(), UBO_NAME_LIGHT_POINT);
    auto index2 = glGetUniformBlockIndex(state->mRenderTime.mProgram->GetUseID(), UBO_NAME_LIGHT_SPOT);
    glUniformBlockBinding(state->mRenderTime.mProgram->GetUseID(), index0, UniformBlockEnum::kLIGHT_DIRECT);
    glUniformBlockBinding(state->mRenderTime.mProgram->GetUseID(), index1, UniformBlockEnum::kLIGHT_POINT);
    glUniformBlockBinding(state->mRenderTime.mProgram->GetUseID(), index2, UniformBlockEnum::kLIGHT_SPOT);

    glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_DIRECT, state->mLightUBO[Light::kDIRECT]);
    glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_POINT, state->mLightUBO[Light::kPOINT]);
    glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_SPOT, state->mLightUBO[Light::kSPOT]);

    auto count0 = std::min(state->mLightQueues.at(Light::kDIRECT).size(), LIMIT_LIGHT_DIRECT);
    auto count1 = std::min(state->mLightQueues.at(Light::kPOINT).size(), LIMIT_LIGHT_POINT);
    auto count2 = std::min(state->mLightQueues.at(Light::kSPOT).size(), LIMIT_LIGHT_SPOT);
    state->mRenderTime.mProgram->BindUniformNumber(UNIFORM_LIGHT_COUNT_DIRECT_, count0);
    state->mRenderTime.mProgram->BindUniformNumber(UNIFORM_LIGHT_COUNT_POINT_, count1);
    state->mRenderTime.mProgram->BindUniformNumber(UNIFORM_LIGHT_COUNT_SPOT_, count2);
    for (auto i = 0, n = 0; i != count0; ++i, ++n)
    {
        state->mRenderTime.mProgram->BindUniformTex2D(SFormat(UNIFORM_SHADOW_MAP_DIRECT_, n).c_str(),
                                                      state->mShadowMap.mDirectTexture[i],
                                                      state->mRenderTime.mTexBase++);
    }
    for (auto i = 0, n = 0; i != count1; ++i, ++n)
    {
        state->mRenderTime.mProgram->BindUniformTex3D(SFormat(UNIFORM_SHADOW_MAP_POINT_, n).c_str(),
                                                      state->mShadowMap.mPointTexture[i],
                                                      state->mRenderTime.mTexBase++);
    }
    for (auto i = 0, n = 0; i != count2; ++i, ++n)
    {
        state->mRenderTime.mProgram->BindUniformTex2D(SFormat(UNIFORM_SHADOW_MAP_SPOT_, n).c_str(),
                                                      state->mShadowMap.mSpotTexture[i],
                                                      state->mRenderTime.mTexBase++);
    }
    state->mRenderTime.mProgram->BindUniformTex2D(UNIFORM_SCREEN_SAO, 
                                                  state->mSSAO.mOcclusionTexture1, 
                                                  state->mRenderTime.mTexBase++);
}
