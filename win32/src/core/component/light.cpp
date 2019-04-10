#include "light.h"
#include "transform.h"
#include "render_target.h"
#include "../mmc.h"
#include "../asset/file.h"
#include "../asset/shader.h"
#include "../render/render.h"
#include "../asset/asset_cache.h"

void Light::OnAdd()
{
	mmc::mRender.AddLight(this);
}

void Light::OnDel()
{
	mmc::mRender.DelLight(this);
}

LightDirect::~LightDirect()
{
	delete mShadowTex;
}

void LightDirect::OpenShadow(
	const std::uint32_t depthW,
	const std::uint32_t depthH,
	const glm::vec2 & orthoX,
	const glm::vec2 & orthoY,
	const glm::vec2 & orthoZ)
{
	_depthW     = depthW; 
	_depthH     = depthH;
    _proj       = glm::ortho(orthoX.x, orthoX.y, orthoY.x, orthoY.y, orthoZ.x, orthoZ.y);
	mShadowTex  = RenderTarget::CreateTexture2D(_depthW, _depthH, RenderTarget::kDEPTH, GL_NONE, GL_NONE, GL_UNSIGNED_BYTE);

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
    assert(mShadowTex != nullptr);
    if (0 == count)
    {
        _position   = GetOwner()->GetTransform()->GetWorldPosition();
        auto up     = mNormal.y > 0.999999f 
                    ? glm::vec3(0, 0, 1) 
                    : glm::vec3(0, 1, 0);
        auto right  = glm::cross(up, mNormal);
        up          = glm::cross(mNormal, right);
	    auto view   = glm::lookAt(_position, _position + mNormal, up);
        auto matrix = _proj * view;

        glBindBuffer(GL_UNIFORM_BUFFER, _blockID);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mMatrix), sizeof(UBOData::mMatrix), &matrix);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mNormal), sizeof(UBOData::mNormal), &mNormal);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mAmbient), sizeof(UBOData::mAmbient), &mAmbient);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mDiffuse), sizeof(UBOData::mDiffuse), &mDiffuse);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mSpecular), sizeof(UBOData::mSpecular), &mSpecular);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glViewport(0, 0, _depthW, _depthH);
        mmc::mRender.GetMatrix().Identity(RenderMatrix::kVIEW);
        mmc::mRender.GetMatrix().Identity(RenderMatrix::kPROJ);
        mmc::mRender.GetMatrix().Mul(RenderMatrix::kVIEW, view);
        mmc::mRender.GetMatrix().Mul(RenderMatrix::kPROJ, _proj);
        rt->BindAttachment(RenderTarget::AttachmentType::kDEPTH, RenderTarget::TextureType::k2D, mShadowTex->GetGLID());
    }
    else
    {
        mmc::mRender.GetMatrix().Pop(RenderMatrix::kPROJ);
        mmc::mRender.GetMatrix().Pop(RenderMatrix::kVIEW);
    }
    return count == 0;
}

LightPoint::~LightPoint()
{
    delete mShadowTex;
}

void LightPoint::OpenShadow(const std::uint32_t depthW, const std::uint32_t depthH, const float n, const float f)
{
	_depthW     = depthW;
	_depthH     = depthH;
	_proj       = glm::perspective(glm::radians(90.0f), (float)_depthW / (float)_depthH, n, f);
	mShadowTex  = RenderTarget::CreateTexture3D(_depthW, _depthH, RenderTarget::kDEPTH, GL_NONE, GL_NONE, GL_UNSIGNED_BYTE);

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
        glViewport(0, 0, _depthW, _depthH);

        _position = GetOwner()->GetTransform()->GetWorldPosition();

        glBindBuffer(GL_UNIFORM_BUFFER, _blockID);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mK0), sizeof(UBOData::mK0), &mK0);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mK1), sizeof(UBOData::mK1), &mK1);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mK2), sizeof(UBOData::mK1), &mK2);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mAmbient), sizeof(UBOData::mAmbient), &mAmbient);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mDiffuse), sizeof(UBOData::mDiffuse), &mDiffuse);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mSpecular), sizeof(UBOData::mSpecular), &mSpecular);
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mPosition), sizeof(UBOData::mPosition), &_position);
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

        auto view = glm::lookAt(_position,
                    std::get<0>(s_faceInfo[count]) + _position,
                    std::get<1>(s_faceInfo[count]));

        mmc::mRender.GetMatrix().Identity(RenderMatrix::kVIEW);
        mmc::mRender.GetMatrix().Identity(RenderMatrix::kPROJ);
        mmc::mRender.GetMatrix().Mul(RenderMatrix::kVIEW, view);
        mmc::mRender.GetMatrix().Mul(RenderMatrix::kPROJ, _proj);
    }
    return count != 7;
}

LightSpot::~LightSpot()
{
    delete mShadowTex;
}

void LightSpot::OpenShadow(const std::uint32_t depthW, const std::uint32_t depthH, const float n, const float f)
{
	_depthW     = depthW; 
    _depthH     = depthH;
    _proj       = glm::perspective(glm::radians(90.0f), (float)_depthW / (float)_depthH, n, f);
	mShadowTex  = RenderTarget::CreateTexture2D(_depthW, _depthH, RenderTarget::kDEPTH, GL_NONE, GL_NONE, GL_UNSIGNED_BYTE);

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
        _position   = GetOwner()->GetTransform()->GetWorldPosition();
        auto up     = mNormal.y > 0.999999f
                    ? glm::vec3(0, 0, 1)
                    : glm::vec3(0, 1, 0);
        auto right  = glm::cross(up, mNormal);
        up          = glm::cross(mNormal, right);
        auto view   = glm::lookAt(_position, _position + mNormal, up);
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
        glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBOData, mPosition), sizeof(UBOData::mPosition), &_position);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
    else
    {
        mmc::mRender.GetMatrix().Pop(RenderMatrix::kPROJ);
        mmc::mRender.GetMatrix().Pop(RenderMatrix::kVIEW);
    }
    return count == 0;
}

