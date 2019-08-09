#include "pipe.h"
#include "../component/light.h"
#include "../cfg/cfg_manager.h"

PipeState::PipeState()
{
    memset(&mSSAO, 0, sizeof(mSSAO));
    memset(&mGBuffer, 0, sizeof(mGBuffer));
    memset(&mLightUBO, 0, sizeof(mLightUBO));
    memset(&mShadowMap, 0, sizeof(mShadowMap));
    memset(&mRenderTime, 0, sizeof(mRenderTime));
    memset(&mPostScreen, 0, sizeof(mPostScreen));

    auto windowW = Global::Ref().RefCfgManager().At("init", "window", "w")->ToInt();
    auto windowH = Global::Ref().RefCfgManager().At("init", "window", "h")->ToInt();

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
    mRenderTarget[1].Start(RenderTarget::BindType::kALL);
    mRenderTarget[1].BindAttachment(RenderTarget::AttachmentType::kCOLOR0, RenderTarget::TextureType::k2D, mPostScreen.mColorTexture);
    mRenderTarget[1].BindAttachment(RenderTarget::AttachmentType::kDEPTH,  RenderTarget::TextureType::k2D, mPostScreen.mDepthTexture);
    mRenderTarget[1].Ended();
}

PipeState::~PipeState()
{
    glDeleteTextures(LIMIT_LIGHT_DIRECT, mShadowMap.mDirectTexture);
    glDeleteTextures(LIMIT_LIGHT_POINT, mShadowMap.mPointTexture);
    glDeleteTextures(LIMIT_LIGHT_SPOT, mShadowMap.mSpotTexture);

    glDeleteTextures(3, &mGBuffer.mPositionTexture);

    glDeleteTextures(1, &mPostScreen.mColorTexture);
    glDeleteTextures(1, &mPostScreen.mDepthTexture);
    glDeleteTextures(2, &mSSAO.mOcclusionTexture0);

    glDeleteBuffers(3, mLightUBO);
}
