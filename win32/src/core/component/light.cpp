#include "light.h"
#include "transform.h"
#include "render_target.h"
#include "../mmc.h"
#include "../asset/file.h"
#include "../asset/shader.h"
#include "../render/render.h"
#include "../asset/asset_cache.h"

Light::Light(LightType type): _type(type)
{
}

Light::~Light()
{
}

void Light::OnAdd()
{
	mmc::mRender.AddLight(this);
}

void Light::OnDel()
{
	mmc::mRender.DelLight(this);
}

void Light::OnUpdate(float dt)
{
}

LightDirect::~LightDirect()
{
	HideShadow();
}

void LightDirect::OpenShadow(const std::uint32_t depthW, const std::uint32_t depthH,
							 const float orthoXMin, const float orthoXMax,
							 const float orthoYMin, const float orthoYMax,
							 const float orthoZMin, const float orthoZMax,
							 const glm::vec3 &up)
{
	_up = up; _depthW = depthW; _depthH = depthH;
	_orthoX.x = orthoXMin; _orthoX.y = orthoXMax;
	_orthoY.x = orthoYMin; _orthoY.y = orthoYMax;
	_orthoZ.x = orthoZMin; _orthoZ.y = orthoZMax;
	assert(_shadowRT == nullptr);
	assert(_shadowTex == nullptr);
	_shadowTex = RenderTarget::Create2DTexture(
		_depthW, _depthH, RenderTarget::kDEPTH);
	_shadowRT = new RenderTarget();
	_shadowRT->Beg();
	_shadowRT->BindAttachment(RenderTarget::kDEPTH, 
		 RenderTarget::k2D, _shadowTex->GetGLID());
	_shadowRT->CloseDraw();
	_shadowRT->CloseRead();
	_shadowRT->End();
}

void LightDirect::HideShadow()
{
	delete _shadowTex; _shadowTex = nullptr;
	delete _shadowRT; _shadowRT = nullptr;
}

void LightDirect::DrawShadow()
{
	if (_shadowTex != nullptr)
	{
		auto world = GetOwner()->GetTransform()->GetWorldPosition();

		auto proj = glm::ortho(_orthoX.x, _orthoX.y,
							   _orthoY.x, _orthoY.y,
							   _orthoZ.x, _orthoZ.y);

		auto view = glm::lookAt(world, world + mNormal, _up);

		_shadowMat = proj * view;

		glViewport(0, 0, _depthW, _depthH);
		mmc::mRender.GetMatrix().Identity(RenderMatrix::kVIEW);
		mmc::mRender.GetMatrix().Identity(RenderMatrix::kPROJ);
		mmc::mRender.GetMatrix().Mul(RenderMatrix::kVIEW, view);
		mmc::mRender.GetMatrix().Mul(RenderMatrix::kPROJ, proj);

		_shadowRT->Beg();
		glCullFace(GL_FRONT);
		mmc::mRender.OnRenderCamera(nullptr);
		glCullFace(GL_BACK);
		_shadowRT->End();

		mmc::mRender.GetMatrix().Pop(RenderMatrix::kPROJ);
		mmc::mRender.GetMatrix().Pop(RenderMatrix::kVIEW);
	}
}

void LightPoint::OpenShadow(const std::uint32_t depthW, const std::uint32_t depthH, const float n, const float f)
{
	assert(_shadowTex == nullptr);
	_n = n; _f = f;
	_depthW = depthW;
	_depthH = depthH;
	_proj = glm::perspective(glm::radians(90.0f), (float)_depthW / (float)_depthH, _n, _f);

	_shadowTex = RenderTarget::Create3DTexture(_depthW, _depthH, RenderTarget::kDEPTH);

	_shadowRT = new RenderTarget();
	_shadowRT->Beg();
	_shadowRT->BindAttachment(RenderTarget::AttachmentType::kDEPTH, 
							  RenderTarget::TextureType::k3D_RIGHT, 
							  _shadowTex->GetGLID());
	_shadowRT->CloseDraw();
	_shadowRT->CloseRead();
	_shadowRT->End();
}

void LightPoint::HideShadow()
{
	delete _shadowTex; _shadowTex = nullptr;
	delete _shadowRT; _shadowRT = nullptr;
}

void LightPoint::DrawShadow()
{
	if (_shadowTex != nullptr)
	{
		glm::mat4 view;

		glViewport(0, 0, _depthW, _depthH);

		auto world = GetOwner()->GetTransform()->GetWorldPosition();
		//	右
		view = glm::lookAt(world, world + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
		DrawShadow(RenderTarget::TextureType::k3D_RIGHT, view);
		//	左
		view = glm::lookAt(world, world + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
		DrawShadow(RenderTarget::TextureType::k3D_LEFT, view);
		//	上
		view = glm::lookAt(world, world + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
		DrawShadow(RenderTarget::TextureType::k3D_TOP, view);
		//	下
		view = glm::lookAt(world, world + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
		DrawShadow(RenderTarget::TextureType::k3D_BOTTOM, view);
		//	前
		view = glm::lookAt(world, world + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
		DrawShadow(RenderTarget::TextureType::k3D_FRONT, view);
		//	后
		view = glm::lookAt(world, world + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
		DrawShadow(RenderTarget::TextureType::k3D_BACK, view);
	}
}

void LightPoint::DrawShadow(size_t idx, const glm::mat4 & view)
{
	_shadowMat[idx - RenderTarget::TextureType::k3D_RIGHT] = _proj * view;

	mmc::mRender.GetMatrix().Identity(RenderMatrix::kVIEW);
	mmc::mRender.GetMatrix().Identity(RenderMatrix::kPROJ);
	mmc::mRender.GetMatrix().Mul(RenderMatrix::kVIEW, view);
	mmc::mRender.GetMatrix().Mul(RenderMatrix::kPROJ, _proj);

	_shadowRT->Beg();
	_shadowRT->BindAttachment(RenderTarget::AttachmentType::kDEPTH, 
							  (RenderTarget::TextureType)idx,
							  _shadowTex->GetGLID());
	glCullFace(GL_FRONT);
	mmc::mRender.OnRenderCamera(nullptr);
	glCullFace(GL_BACK);
	_shadowRT->End();

	mmc::mRender.GetMatrix().Pop(RenderMatrix::kPROJ);
	mmc::mRender.GetMatrix().Pop(RenderMatrix::kVIEW);
}

void LightSpot::OpenShadow(const std::uint32_t depthW, const std::uint32_t depthH, const float n, const float f, const glm::vec3 & up)
{
	_depthW = depthW; _depthH = depthH; _n = n; _f = f; _up = up;
	assert(_shadowRT == nullptr);
	assert(_shadowTex == nullptr);
	_shadowTex = RenderTarget::Create2DTexture(
		_depthW, _depthH, RenderTarget::kDEPTH);
	_shadowRT = new RenderTarget();
	_shadowRT->Beg();
	_shadowRT->BindAttachment(RenderTarget::kDEPTH,
		 RenderTarget::k2D, _shadowTex->GetGLID());
	_shadowRT->CloseDraw();
	_shadowRT->CloseRead();
	_shadowRT->End();
}

void LightSpot::HideShadow()
{
	delete _shadowTex; _shadowTex = nullptr;
	delete _shadowRT; _shadowRT = nullptr;
}

void LightSpot::DrawShadow()
{
	if (_shadowTex != nullptr)
	{
		auto proj = glm::perspective(glm::radians(90.0f), (float)_depthW / (float)_depthH, _n, _f);
		auto world = GetOwner()->GetTransform()->GetWorldPosition();
		auto view = glm::lookAt(world, world + mNormal, _up);

		_shadowMat = proj * view;

		glViewport(0, 0, _depthW, _depthH);
		mmc::mRender.GetMatrix().Identity(RenderMatrix::kVIEW);
		mmc::mRender.GetMatrix().Identity(RenderMatrix::kPROJ);
		mmc::mRender.GetMatrix().Mul(RenderMatrix::kVIEW, view);
		mmc::mRender.GetMatrix().Mul(RenderMatrix::kPROJ, proj);

		_shadowRT->Beg();
		glCullFace(GL_FRONT);
		mmc::mRender.OnRenderCamera(nullptr);
		glCullFace(GL_BACK);
		_shadowRT->End();

		mmc::mRender.GetMatrix().Pop(RenderMatrix::kPROJ);
		mmc::mRender.GetMatrix().Pop(RenderMatrix::kVIEW);
	}
}
