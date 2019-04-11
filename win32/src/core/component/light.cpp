#include "light.h"
#include "transform.h"
#include "../mmc.h"
#include "../asset/file.h"
#include "../asset/shader.h"
#include "../render/render.h"
#include "../asset/asset_cache.h"

uint Light::s_VIEW_W = 0;
uint Light::s_VIEW_H = 0;
Light::TexPool Light::s_texPool;

void Light::TexPool::Clear()
{
    glDeleteTextures(1, &_tex2D);
    glDeleteTextures(1, &_tex3D);
    _len2D = 0; _len3D = 0;
    _tex2D = 0; _tex3D = 0;
    _texOrder2Ds.clear();
    _texOrder3Ds.clear();
}

uint Light::TexPool::GetTexture2D()
{
    return _tex2D;
}

uint Light::TexPool::GetTexture3D()
{
    return _tex3D;
}

uint Light::TexPool::GetTexOrder2D()
{
    if (_texOrder2Ds.empty())
    {
        AllocTexOrder2D();
    }
    auto top = _texOrder2Ds.back();
    _texOrder2Ds.pop_back();
    return top;
}

uint Light::TexPool::GetTexOrder3D()
{
    if (_texOrder3Ds.empty())
    {
        AllocTexOrder3D();
    }
    auto top = _texOrder3Ds.back();
    _texOrder3Ds.pop_back();
    return top;
}

void Light::TexPool::FreeTexOrder2D(uint id)
{
    _texOrder2Ds.push_back(id);

    if (_texOrder2Ds.size() == _len2D)
    {
        glDeleteTextures(1, &_tex2D);
        _tex2D = 0; _len2D = 1;
        _texOrder2Ds.clear();
    }
}

void Light::TexPool::FreeTexOrder3D(uint id)
{
    _texOrder3Ds.push_back(id);

    if (_texOrder3Ds.size() == _len3D)
    {
        glDeleteTextures(1, &_tex3D);
        _tex3D = 0; _len3D = 1;
        _texOrder3Ds.clear();
    }
}

void Light::TexPool::AllocTexOrder2D()
{
    if ((_len2D *= 2) == 0)
    {
        glGenTextures(1, &_len2D);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, _tex2D);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, s_VIEW_W, s_VIEW_H, _len2D);
    for (auto i = 0; i != _len2D; ++i)
    {
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, s_VIEW_W, s_VIEW_H, 1,
                            GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
    }
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void Light::TexPool::AllocTexOrder3D()
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
    _proj       = glm::ortho(orthoX.x, orthoX.y, orthoY.x, orthoY.y, orthoZ.x, orthoZ.y);

    //  环境光, 漫反射, 镜面反射, 法线, 矩阵
    if (_blockID == 0) 
    {
        glGenBuffers(1, &_blockID); 
    }
    glBindBuffer(GL_UNIFORM_BUFFER, _blockID);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UBOData), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

bool LightDirect::NextDrawShadow(size_t count, RenderTarget * rt)
{
    if (0 == count)
    {
        auto pos    = GetOwner()->GetTransform()->GetWorldPosition();
        auto up     = mNormal.y > 0.999999f 
                    ? glm::vec3(0, 0, 1) 
                    : glm::vec3(0, 1, 0);
        auto right  = glm::cross(up, mNormal);
        up          = glm::cross(mNormal, right);
	    auto view   = glm::lookAt(pos, pos + mNormal, up);
        auto matrix = _proj * view;

        glBindBuffer(GL_UNIFORM_BUFFER, _blockID);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mMatrix), sizeof(UBOData::mMatrix), &matrix);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mNormal), sizeof(UBOData::mNormal), &mNormal);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mAmbient), sizeof(UBOData::mAmbient), &mAmbient);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mDiffuse), sizeof(UBOData::mDiffuse), &mDiffuse);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mSpecular), sizeof(UBOData::mSpecular), &mSpecular);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mPosition), sizeof(UBOData::mSpecular), &pos);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glViewport(0, 0, Light::s_VIEW_W, Light::s_VIEW_H);
        mmc::mRender.GetMatrix().Identity(RenderMatrix::kVIEW);
        mmc::mRender.GetMatrix().Identity(RenderMatrix::kPROJ);
        mmc::mRender.GetMatrix().Mul(RenderMatrix::kVIEW, view);
        mmc::mRender.GetMatrix().Mul(RenderMatrix::kPROJ, _proj);
        rt->BindAttachment(RenderTarget::AttachmentType::kDEPTH, RenderTarget::TextureType::k2D, Light::s_texPool.GetTexture2D(), _texOrder);
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
	_proj       = glm::perspective(glm::radians(90.0f), (float)Light::s_VIEW_W / (float)Light::s_VIEW_W, n, f);

    //  光线衰减系数k0, k1, k2, 环境光, 漫反射, 镜面反射, 世界坐标
    if (_blockID == 0) 
    { 
        glGenBuffers(1, &_blockID); 
    }
    glBindBuffer(GL_UNIFORM_BUFFER, _blockID);
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

        glBindBuffer(GL_UNIFORM_BUFFER, _blockID);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mK0), sizeof(UBOData::mK0), &mK0);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mK1), sizeof(UBOData::mK1), &mK1);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mK2), sizeof(UBOData::mK1), &mK2);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mAmbient), sizeof(UBOData::mAmbient), &mAmbient);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mDiffuse), sizeof(UBOData::mDiffuse), &mDiffuse);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mSpecular), sizeof(UBOData::mSpecular), &mSpecular);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mPosition), sizeof(UBOData::mPosition), &_pos);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    if (count < 7)
    {
        static std::tuple<glm::vec3, glm::vec3, RenderTarget::TextureType> s_faceInfo[6] = {
            {   glm::vec3( 1,  0,  0), glm::vec3(0, -1,  0), RenderTarget::TextureType::k3D_RIGHT     },
            {   glm::vec3(-1,  0,  0), glm::vec3(0, -1,  0), RenderTarget::TextureType::k3D_LEFT      },
            {   glm::vec3( 0,  1,  0), glm::vec3(0,  0,  1), RenderTarget::TextureType::k3D_TOP       },
            {   glm::vec3( 0, -1,  0), glm::vec3(0,  0, -1), RenderTarget::TextureType::k3D_BOTTOM    },
            {   glm::vec3( 0,  0,  1), glm::vec3(0, -1,  0), RenderTarget::TextureType::k3D_FRONT     },
            {   glm::vec3( 0,  0, -1), glm::vec3(0, -1,  0), RenderTarget::TextureType::k3D_BACK      },
        };

        auto view = glm::lookAt(_pos,
                    std::get<0>(s_faceInfo[count]) + _pos,
                    std::get<1>(s_faceInfo[count]));

        mmc::mRender.GetMatrix().Identity(RenderMatrix::kVIEW);
        mmc::mRender.GetMatrix().Identity(RenderMatrix::kPROJ);
        mmc::mRender.GetMatrix().Mul(RenderMatrix::kVIEW, view);
        mmc::mRender.GetMatrix().Mul(RenderMatrix::kPROJ, _proj);
        rt->BindAttachment(RenderTarget::AttachmentType::kDEPTH, std::get<2>(s_faceInfo[count]), Light::s_texPool.GetTexture3D(), _texOrder);
    }
    return count != 7;
}

void LightSpot::OpenShadow(const float n, const float f)
{
    _proj       = glm::perspective(glm::radians(90.0f), (float)Light::s_VIEW_W / (float)Light::s_VIEW_H, n, f);

    //  光线衰减系数k0, k1, k2, 内锥, 外锥, 环境光, 漫反射, 镜面反射, 世界坐标
    if (_blockID == 0)
    {
        glGenBuffers(1, &_blockID);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, _blockID);
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
        auto matrix = _proj * view;

        glBindBuffer(GL_UNIFORM_BUFFER, _blockID);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mK0), sizeof(UBOData::mK0), &mK0);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mK1), sizeof(UBOData::mK1), &mK1);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mK2), sizeof(UBOData::mK1), &mK2);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mInCone), sizeof(UBOData::mInCone), &mInCone);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mOutCone), sizeof(UBOData::mOutCone), &mOutCone);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mAmbient), sizeof(UBOData::mAmbient), &mAmbient);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mDiffuse), sizeof(UBOData::mDiffuse), &mDiffuse);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mSpecular), sizeof(UBOData::mSpecular), &mSpecular);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mPosition), sizeof(UBOData::mPosition), &_pos);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        mmc::mRender.GetMatrix().Identity(RenderMatrix::kVIEW);
        mmc::mRender.GetMatrix().Identity(RenderMatrix::kPROJ);
        mmc::mRender.GetMatrix().Mul(RenderMatrix::kVIEW, view);
        mmc::mRender.GetMatrix().Mul(RenderMatrix::kPROJ, _proj);
        rt->BindAttachment(RenderTarget::AttachmentType::kDEPTH, RenderTarget::TextureType::k2D, Light::s_texPool.GetTexture2D(), _texOrder);
    }
    else
    {
        mmc::mRender.GetMatrix().Pop(RenderMatrix::kPROJ);
        mmc::mRender.GetMatrix().Pop(RenderMatrix::kVIEW);
    }
    return count == 0;
}

