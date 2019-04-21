#include "light.h"
#include "transform.h"
#include "../mmc.h"
#include "../asset/file.h"
#include "../asset/shader.h"
#include "../render/render.h"
#include "../asset/asset_cache.h"

uint Light::s_VIEW_W = 0;
uint Light::s_VIEW_H = 0;
Light::ShadowMapPool Light::s_shadowMapPool;

void Light::ShadowMapPool::Clear()
{
    glDeleteTextures(1, &_tex2D);
    glDeleteTextures(1, &_tex3D);
    _len2D = 0; _len3D = 0;
    _tex2D = 0; _tex3D = 0;
    _posStock2D.clear();
    _posStock3D.clear();
}

uint Light::ShadowMapPool::GetTex2D()
{
    return _tex2D;
}

uint Light::ShadowMapPool::GetTex3D()
{
    return _tex3D;
}

uint Light::ShadowMapPool::GetPos2D()
{
    AllocPos2D();

    auto top = _posStock2D.back();
    _posStock2D.pop_back();
    return top;
}

uint Light::ShadowMapPool::GetPos3D()
{
    AllocPos3D();

    auto top = _posStock3D.back();
    _posStock3D.pop_back();
    return top;
}

void Light::ShadowMapPool::FreePos2D(uint id)
{
    _posStock2D.push_back(id);

    if (_posStock2D.size() == _len2D)
    {
        glDeleteTextures(1, &_tex2D);
        _tex2D = 0; _len2D = 1;
        _posStock2D.clear();
    }
}

void Light::ShadowMapPool::FreePos3D(uint id)
{
    _posStock3D.push_back(id);

    if (_posStock3D.size() == _len3D)
    {
        glDeleteTextures(1, &_tex3D);
        _tex3D = 0; _len3D = 1;
        _posStock3D.clear();
    }
}

void Light::ShadowMapPool::AllocPos2D()
{
    if (_tex2D == 0)
    {
        glGenTextures(1, &_tex2D);
    }
    if (_posStock2D.empty())
    {
        std::generate_n(std::back_inserter(_posStock2D), 
                        ShadowMapPool::s_LEN_STEP, 
                        [this]() { return _len2D++; });

        glBindTexture(GL_TEXTURE_2D_ARRAY, _tex2D);
        for (auto i = 0; i != _len2D; ++i)
        {
            glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, s_VIEW_W, s_VIEW_H, 
                              _len2D, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
        }
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }
}

void Light::ShadowMapPool::AllocPos3D()
{

}

//  --------------------------------------------------------------------------------
//  光源实现
void Light::OnAdd()
{
	mmc::mRender.AddLight(this);
}

void Light::OnDel()
{
	mmc::mRender.DelLight(this);
}

void LightDirect::OpenShadow(
	const glm::vec2 & orthoX,
	const glm::vec2 & orthoY,
	const glm::vec2 & orthoZ)
{
    _proj = glm::ortho(orthoX.x, orthoX.y, orthoY.x, orthoY.y, orthoZ.x, orthoZ.y);

    //  环境光, 漫反射, 镜面反射, 法线, 矩阵
    if (_ubo == 0) 
    {
        glGenBuffers(1, &_ubo);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UBOData), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

bool LightDirect::NextDrawShadow(size_t count, RenderTarget * rt)
{
    if (0 == count)
    {
        _pos        = GetOwner()->GetTransform()->GetWorldPosition();
        auto up     = mNormal.y > 0.999999f 
                    ? glm::vec3(0, 0, 1) 
                    : glm::vec3(0, 1, 0);
        auto right  = glm::cross(up, mNormal);
        up          = glm::cross(mNormal, right);
        auto view   = glm::lookAt(_pos, _pos + mNormal, up);
        _matrix     = _proj * view;

        glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mSMP),             sizeof(UBOData::mSMP),          &_smp);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mMatrix),          sizeof(UBOData::mMatrix),       &_matrix);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mNormal),          sizeof(UBOData::mNormal),       &mNormal);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mAmbient),         sizeof(UBOData::mAmbient),      &mAmbient);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mDiffuse),         sizeof(UBOData::mDiffuse),      &mDiffuse);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mSpecular),        sizeof(UBOData::mSpecular),     &mSpecular);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mPosition),        sizeof(UBOData::mSpecular),     &_pos);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glViewport(0, 0, Light::s_VIEW_W, Light::s_VIEW_H);
        mmc::mRender.GetMatrix().Identity(RenderMatrix::kVIEW);
        mmc::mRender.GetMatrix().Identity(RenderMatrix::kPROJ);
        mmc::mRender.GetMatrix().Mul(RenderMatrix::kVIEW, view);
        mmc::mRender.GetMatrix().Mul(RenderMatrix::kPROJ, _proj);
        rt->BindAttachment(RenderTarget::AttachmentType::kDEPTH, 
                           RenderTarget::TextureType::k2D_ARRAY, 
                           0, Light::s_shadowMapPool.GetTex2D(), _smp);
    }
    else
    {
        mmc::mRender.GetMatrix().Pop(RenderMatrix::kPROJ);
        mmc::mRender.GetMatrix().Pop(RenderMatrix::kVIEW);
    }
    return count == 0;
}

void LightPoint::OpenShadow(const float n, const float f)
{
	_proj = glm::perspective(glm::radians(90.0f), (float)Light::s_VIEW_W / (float)Light::s_VIEW_W, n, f);

    //  光线衰减系数k0, k1, k2, 环境光, 漫反射, 镜面反射, 世界坐标
    if (_ubo == 0) 
    { 
        glGenBuffers(1, &_ubo); 
    }
    glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UBOData), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

bool LightPoint::NextDrawShadow(size_t count, RenderTarget * rt)
{
    if (count != 0)
    {
        mmc::mRender.GetMatrix().Pop(RenderMatrix::kPROJ);
        mmc::mRender.GetMatrix().Pop(RenderMatrix::kVIEW);
    }
    else
    {
        glViewport(0, 0, Light::s_VIEW_W, Light::s_VIEW_H);

        _pos = GetOwner()->GetTransform()->GetWorldPosition();

        glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mSMP),             sizeof(UBOData::mSMP),          &_smp);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mK0),              sizeof(UBOData::mK0),           &mK0);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mK1),              sizeof(UBOData::mK1),           &mK1);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mK2),              sizeof(UBOData::mK1),           &mK2);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mAmbient),         sizeof(UBOData::mAmbient),      &mAmbient);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mDiffuse),         sizeof(UBOData::mDiffuse),      &mDiffuse);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mSpecular),        sizeof(UBOData::mSpecular),     &mSpecular);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mPosition),        sizeof(UBOData::mPosition),     &_pos);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    if (count < 7)
    {
        static std::tuple<glm::vec3, glm::vec3> s_faceInfo[6] = {
            { glm::vec3( 1,  0,  0), glm::vec3(0, -1,  0) },
            { glm::vec3(-1,  0,  0), glm::vec3(0, -1,  0) },
            { glm::vec3( 0,  1,  0), glm::vec3(0,  0,  1) },
            { glm::vec3( 0, -1,  0), glm::vec3(0,  0, -1) },
            { glm::vec3( 0,  0,  1), glm::vec3(0, -1,  0) },
            { glm::vec3( 0,  0, -1), glm::vec3(0, -1,  0) },
        };

        auto view = glm::lookAt(_pos,
                    std::get<0>(s_faceInfo[count]) + _pos,
                    std::get<1>(s_faceInfo[count]));

        mmc::mRender.GetMatrix().Identity(RenderMatrix::kVIEW);
        mmc::mRender.GetMatrix().Identity(RenderMatrix::kPROJ);
        mmc::mRender.GetMatrix().Mul(RenderMatrix::kVIEW, view);
        mmc::mRender.GetMatrix().Mul(RenderMatrix::kPROJ, _proj);
        rt->BindAttachment(RenderTarget::AttachmentType::kDEPTH, 
                           RenderTarget::TextureType::k3D_ARRAY, 
                           count, Light::s_shadowMapPool.GetTex3D(), _smp);
    }
    return count != 7;
}

void LightSpot::OpenShadow(const float n, const float f)
{
    _proj = glm::perspective(glm::radians(90.0f), (float)Light::s_VIEW_W / (float)Light::s_VIEW_H, n, f);

    //  光线衰减系数k0, k1, k2, 内锥, 外锥, 环境光, 漫反射, 镜面反射, 世界坐标
    if (_ubo == 0)
    {
        glGenBuffers(1, &_ubo);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UBOData), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

bool LightSpot::NextDrawShadow(size_t count, RenderTarget * rt)
{
    if (count == 0)
    {
        _pos        = GetOwner()->GetTransform()->GetWorldPosition();
        auto up     = mNormal.y > 0.999999f
                    ? glm::vec3(0, 0, 1)
                    : glm::vec3(0, 1, 0);
        auto right  = glm::cross(up, mNormal);
        up          = glm::cross(mNormal, right);
        auto view   = glm::lookAt(_pos, _pos + mNormal, up);
        _matrix     = _proj * view;

        glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mSMP),             sizeof(UBOData::mSMP),          &_smp);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mK0),              sizeof(UBOData::mK0),           &mK0);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mK1),              sizeof(UBOData::mK1),           &mK1);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mK2),              sizeof(UBOData::mK1),           &mK2);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mInCone),          sizeof(UBOData::mInCone),       &mInCone);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mOutCone),         sizeof(UBOData::mOutCone),      &mOutCone);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mAmbient),         sizeof(UBOData::mAmbient),      &mAmbient);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mDiffuse),         sizeof(UBOData::mDiffuse),      &mDiffuse);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mSpecular),        sizeof(UBOData::mSpecular),     &mSpecular);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mPosition),        sizeof(UBOData::mPosition),     &_pos);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        mmc::mRender.GetMatrix().Identity(RenderMatrix::kVIEW);
        mmc::mRender.GetMatrix().Identity(RenderMatrix::kPROJ);
        mmc::mRender.GetMatrix().Mul(RenderMatrix::kVIEW, view);
        mmc::mRender.GetMatrix().Mul(RenderMatrix::kPROJ, _proj);
        rt->BindAttachment(RenderTarget::AttachmentType::kDEPTH, 
                           RenderTarget::TextureType::k2D_ARRAY, 
                           0, Light::s_shadowMapPool.GetTex2D(), _smp);
    }
    else
    {
        mmc::mRender.GetMatrix().Pop(RenderMatrix::kPROJ);
        mmc::mRender.GetMatrix().Pop(RenderMatrix::kVIEW);
    }
    return count == 0;
}

