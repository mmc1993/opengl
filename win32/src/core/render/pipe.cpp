#include "pipe.h"
#include "../component/light.h"
#include "../cfg/cfg_manager.h"

inline PipeState::PipeState()
{
    memset(&mSSAO, 0, sizeof(mSSAO));
    memset(&mGBuffer, 0, sizeof(mGBuffer));
    memset(&mLightUBO, 0, sizeof(mLightUBO));
    memset(&mShadowMap, 0, sizeof(mShadowMap));
    memset(&mRenderTime, 0, sizeof(mRenderTime));
    memset(&mPostScreen, 0, sizeof(mPostScreen));

    auto shadowW = Global::Ref().RefCfgManager().At("init", "shadow_map", "w")->ToInt();
    auto shadowH = Global::Ref().RefCfgManager().At("init", "shadow_map", "h")->ToInt();
    auto windowW = Global::Ref().RefCfgManager().At("init", "window", "w")->ToInt();
    auto windowH = Global::Ref().RefCfgManager().At("init", "window", "h")->ToInt();

    //  阴影贴图
    glGenTextures(LIMIT_LIGHT_DIRECT, mShadowMap.mDirectTexture);
    glGenTextures(LIMIT_LIGHT_POINT, mShadowMap.mPointTexture);
    glGenTextures(LIMIT_LIGHT_SPOT, mShadowMap.mSpotTexture);
    //  方向光
    for (auto i = 0; i != LIMIT_LIGHT_DIRECT; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, mShadowMap.mDirectTexture[i]);
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
        glBindTexture(GL_TEXTURE_CUBE_MAP, mShadowMap.mPointTexture[i]);
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
        glBindTexture(GL_TEXTURE_2D, mShadowMap.mSpotTexture[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowW, shadowH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    //  后期屏幕
    glGenTextures(2, &mPostScreen.mColorTexture);

    glBindTexture(GL_TEXTURE_2D, mPostScreen.mColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowW, windowH, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, mPostScreen.mDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowW, windowH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    //  绑定渲染目标
    mRenderTarget[1].Start();
    mRenderTarget[1].BindAttachment(RenderTarget::AttachmentType::kCOLOR0, RenderTarget::TextureType::k2D, mPostScreen.mColorTexture);
    mRenderTarget[1].BindAttachment(RenderTarget::AttachmentType::kDEPTH,  RenderTarget::TextureType::k2D, mPostScreen.mDepthTexture);
    mRenderTarget[1].Ended();
}
