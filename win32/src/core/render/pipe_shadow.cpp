#include "pipe_shadow.h"
#include "renderer.h"
#include "../cfg/cfg_manager.h"
#include "../component/light.h"

void PipeShadow::OnAdd(Renderer * renderer, PipeState * state)
{
    auto shadowW = Global::Ref().RefCfgManager().At("init", "shadow_map", "w")->ToInt();
    auto shadowH = Global::Ref().RefCfgManager().At("init", "shadow_map", "h")->ToInt();

    //  阴影贴图
    glGenTextures(LIMIT_LIGHT_DIRECT, state->mShadowMap.mDirectTexture);
    glGenTextures(LIMIT_LIGHT_POINT, state->mShadowMap.mPointTexture);
    glGenTextures(LIMIT_LIGHT_SPOT, state->mShadowMap.mSpotTexture);
    //  方向光
    for (auto i = 0; i != LIMIT_LIGHT_DIRECT; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, state->mShadowMap.mDirectTexture[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowW, shadowH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    //  点光源
    for (auto i = 0; i != LIMIT_LIGHT_POINT; ++i)
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, state->mShadowMap.mPointTexture[i]);
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
    //  聚光灯
    for (auto i = 0; i != LIMIT_LIGHT_SPOT; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, state->mShadowMap.mSpotTexture[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowW, shadowH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void PipeShadow::OnDel(Renderer * renderer, PipeState * state)
{
}

void PipeShadow::OnUpdate(Renderer * renderer, PipeState * state)
{
    for (auto i = 0; i != std::min(state->mLightQueues.at(Light::kDIRECT).size(), LIMIT_LIGHT_DIRECT); ++i)
    {
        BakeShadow(renderer, state, state->mShadowMap.mDirectTexture[i], &state->mLightQueues.at(Light::kDIRECT).at(i), &PipeShadow::OnBakeShadow2D);
    }
    for (auto i = 0; i != std::min(state->mLightQueues.at(Light::kPOINT).size(), LIMIT_LIGHT_POINT); ++i)
    {
        BakeShadow(renderer, state, state->mShadowMap.mPointTexture[i], &state->mLightQueues.at(Light::kPOINT).at(i), &PipeShadow::OnBakeShadow3D);
    }
    for (auto i = 0; i != std::min(state->mLightQueues.at(Light::kSPOT).size(), LIMIT_LIGHT_SPOT); ++i)
    {
        BakeShadow(renderer, state, state->mShadowMap.mSpotTexture[i], &state->mLightQueues.at(Light::kSPOT).at(i), &PipeShadow::OnBakeShadow2D);
    }
    ASSERT_LOG(state->mRenderTime.mCamera != nullptr, "Camera Must Not Null");
    glViewport(
        (iint)state->mRenderTime.mCamera->mViewport.x,
        (iint)state->mRenderTime.mCamera->mViewport.y,
        (iint)state->mRenderTime.mCamera->mViewport.z,
        (iint)state->mRenderTime.mCamera->mViewport.w);
}

inline void PipeShadow::BakeShadow(Renderer * renderer, PipeState * state, uint texture, const LightCommand * light, const BakeFunc_t bakefunc)
{
    state->mRenderTarget[0].Start(RenderTarget::BindType::kDRAW);
    glDrawBuffer(RenderTarget::AttachmentType::kNONE);
    (this->*bakefunc)(renderer, state, texture,light);
    state->mRenderTarget[0].Ended();
}

inline void PipeShadow::OnBakeShadow2D(Renderer * renderer, PipeState * state, uint texture, const LightCommand * light)
{
    glViewport(0, 0,
        Global::Ref().RefCfgManager().At("init", "shadow_map", "w")->ToInt(),
        Global::Ref().RefCfgManager().At("init", "shadow_map", "h")->ToInt());
    renderer->GetMatrixStack().Identity(MatrixStack::kVIEW);
    renderer->GetMatrixStack().Identity(MatrixStack::kPROJ);
    renderer->GetMatrixStack().Mul(MatrixStack::kVIEW, light->mView);
    renderer->GetMatrixStack().Mul(MatrixStack::kPROJ, light->mProj);
    state->mRenderTarget[0].BindAttachment(RenderTarget::AttachmentType::kDEPTH, RenderTarget::TextureType::k2D, texture);
    DrawShadow(renderer, state, light);
    renderer->GetMatrixStack().Pop(MatrixStack::kPROJ);
    renderer->GetMatrixStack().Pop(MatrixStack::kVIEW);
}

inline void PipeShadow::OnBakeShadow3D(Renderer * renderer, PipeState * state, uint texture, const LightCommand * light)
{
    static const std::tuple<glm::vec3, glm::vec3> s_faceInfo[6] = {
        { glm::vec3( 1,  0,  0), glm::vec3(0, -1,  0) },
        { glm::vec3(-1,  0,  0), glm::vec3(0, -1,  0) },
        { glm::vec3( 0,  1,  0), glm::vec3(0,  0,  1) },
        { glm::vec3( 0, -1,  0), glm::vec3(0,  0, -1) },
        { glm::vec3( 0,  0,  1), glm::vec3(0, -1,  0) },
        { glm::vec3( 0,  0, -1), glm::vec3(0, -1,  0) },
    };
    glViewport(0, 0,
        Global::Ref().RefCfgManager().At("init", "shadow_map", "w")->ToInt(),
        Global::Ref().RefCfgManager().At("init", "shadow_map", "h")->ToInt());

    for (auto i = 0; i != 6; ++i)
    {
        auto view = glm::lookAt(         light->mPosition,
            std::get<0>(s_faceInfo[i]) + light->mPosition,
            std::get<1>(s_faceInfo[i]));
        renderer->GetMatrixStack().Identity(MatrixStack::kVIEW);
        renderer->GetMatrixStack().Identity(MatrixStack::kPROJ);
        renderer->GetMatrixStack().Mul(MatrixStack::kVIEW, view);
        renderer->GetMatrixStack().Mul(MatrixStack::kPROJ, light->mProj);
        state->mRenderTarget[0].BindAttachment(RenderTarget::AttachmentType::kDEPTH, 
            (RenderTarget::TextureType)(RenderTarget::TextureType::k3D + i), texture);
        DrawShadow(renderer, state, light);
        renderer->GetMatrixStack().Pop(MatrixStack::kPROJ);
        renderer->GetMatrixStack().Pop(MatrixStack::kVIEW);
    }
}

inline void PipeShadow::DrawShadow(Renderer * renderer, PipeState * state, const LightCommand * light)
{
    glClear(GL_DEPTH_BUFFER_BIT);
    for (auto & command : state->mShadowQueue)
    {
        if (renderer->Bind(command.mMaterial->GetProgram(), command.mSubPass))
        {
            renderer->Post(light);
        }
        renderer->Post(command.mProgramParam);
        renderer->Post(&command.mTransform);
        renderer->Post(
            (DrawTypeEnum)command.mMaterial->GetProgram()->GetPass(command.mSubPass).mDrawType,
            (FragTypeEnum)command.mMaterial->GetProgram()->GetPass(command.mSubPass).mFragType,
            command.mMaterial->GetMesh());
    }
}
