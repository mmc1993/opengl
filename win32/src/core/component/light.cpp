#include "light.h"
#include "transform.h"
#include "render_target.h"
#include "../mmc.h"
#include "../asset/file.h"
#include "../asset/shader.h"
#include "../render/render.h"

Light::Light(LightType type): _type(type)
{
	float vertexs[] = {
		 0.1f, -0.1f, -0.1f,
		 0.1f, -0.1f,  0.1f,
		-0.1f, -0.1f,  0.1f,
		-0.1f, -0.1f, -0.1f,
		 0.1f,  0.1f, -0.1f,
		 0.1f,  0.1f,  0.1f,
		-0.1f,  0.1f,  0.1f,
		-0.1f,  0.1f, -0.1f,
	};

	std::uint32_t indices[] = {
		4, 0, 3,
		4, 3, 7,
		2, 6, 7,
		2, 7, 3,
		1, 5, 2,
		5, 6, 2,
		0, 4, 1,
		4, 5, 1,
		4, 7, 5,
		7, 6, 5,
		0, 1, 2,
		0, 2, 3,
	};

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexs), vertexs, GL_STATIC_DRAW);

	glGenBuffers(1, &_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	_shader = File::LoadShader("res/shader/light.shader");
}

Light::~Light()
{
	glDeleteBuffers(1, &_vbo);
	glDeleteBuffers(1, &_ebo);
	glDeleteVertexArrays(1, &_vao);
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
	if (mIsDraw)
	{
		Render::Command command;
		command.mCameraFlag = GetOwner()->GetCameraFlag();
		command.mCallFn = [this]() {
			glEnable(GL_DEPTH_TEST);
			mmc::mRender.Bind(_shader);
			mmc::mRender.RenderIdx(_vao, 36);
			glDisable(GL_DEPTH_TEST);
		};
		mmc::mRender.PostCommand(command);
	}
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

const glm::mat4 LightDirect::GetShadowMat() const
{
	return _shadowMat;
}

const Bitmap * LightDirect::GetShadowTex() const
{
	return _shadowTex;
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
		mmc::mRender.GetMatrix().Identity(Render::Matrix::kVIEW);
		mmc::mRender.GetMatrix().Identity(Render::Matrix::kMODEL);
		mmc::mRender.GetMatrix().Identity(Render::Matrix::kPROJECT);
		mmc::mRender.GetMatrix().Mul(Render::Matrix::kVIEW, view);
		mmc::mRender.GetMatrix().Mul(Render::Matrix::kPROJECT, proj);

		_shadowRT->Beg();
		glCullFace(GL_FRONT);
		mmc::mRender.OnRenderCamera(nullptr);
		glCullFace(GL_BACK);
		_shadowRT->End();

		mmc::mRender.GetMatrix().Pop(Render::Matrix::kPROJECT);
		mmc::mRender.GetMatrix().Pop(Render::Matrix::kVIEW);
		mmc::mRender.GetMatrix().Pop(Render::Matrix::kMODEL);
	}
}

void LightPoint::OpenShadow(const std::uint32_t depthW, const std::uint32_t depthH, const float n, const float f)
{
	assert(_shadowTex == nullptr);
	_depthW = depthW; _depthH = depthH; _n = n; _f = f;
	_shadowTex = RenderTarget::Create3DTexture(_depthW, _depthH, RenderTarget::kDEPTH);
	for (auto i = 0; i != 6; ++i)
	{
		assert(_shadowRT[i] == nullptr);
		auto texType = RenderTarget::TextureType(RenderTarget::k3D_RIGHT + i);
		_shadowRT[i] = new RenderTarget();
		_shadowRT[i]->Beg();
		_shadowRT[i]->BindAttachment(RenderTarget::kDEPTH, texType, _shadowTex->GetGLID());
		_shadowRT[i]->CloseDraw();
		_shadowRT[i]->CloseRead();
		_shadowRT[i]->End();
	}
}

void LightPoint::HideShadow()
{
	for (auto i = 0; i != 6; ++i)
	{
		delete _shadowRT[i]; _shadowRT[i] = nullptr;
	}
	delete _shadowTex; _shadowTex = nullptr;
}

const glm::mat4 LightPoint::GetShadowMat(size_t idx) const
{
	return _shadowMat[idx];
}

BitmapCube * LightPoint::GetShadowTex()
{
	return _shadowTex;
}

void LightPoint::DrawShadow()
{
	if (_shadowTex != nullptr)
	{
		glm::mat4 proj, view;
		
		glViewport(0, 0, _depthW, _depthH);

		auto world = GetOwner()->GetTransform()->GetWorldPosition();

		proj = glm::perspective(glm::radians(90.0f), (float)_depthW / (float)_depthH, _n, _f);

		//	右
		view = glm::lookAt(world, world + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
		DrawShadow(0, proj, view);
		//	左
		view = glm::lookAt(world, world + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
		DrawShadow(1, proj, view);
		//	上
		view = glm::lookAt(world, world + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
		DrawShadow(2, proj, view);
		//	下
		view = glm::lookAt(world, world + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
		DrawShadow(3, proj, view);
		//	前
		view = glm::lookAt(world, world + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
		DrawShadow(4, proj, view);
		//	后
		view = glm::lookAt(world, world + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
		DrawShadow(5, proj, view);
	}
}

void LightPoint::DrawShadow(size_t idx, const glm::mat4 & proj, const glm::mat4 & view)
{
	_shadowMat[idx] = proj * view;

	mmc::mRender.GetMatrix().Identity(Render::Matrix::kVIEW);
	mmc::mRender.GetMatrix().Identity(Render::Matrix::kMODEL);
	mmc::mRender.GetMatrix().Identity(Render::Matrix::kPROJECT);
	mmc::mRender.GetMatrix().Mul(Render::Matrix::kVIEW, view);
	mmc::mRender.GetMatrix().Mul(Render::Matrix::kPROJECT, proj);

	_shadowRT[idx]->Beg();
	glCullFace(GL_FRONT);
	mmc::mRender.OnRenderCamera(nullptr);
	glCullFace(GL_BACK);
	_shadowRT[idx]->End();

	mmc::mRender.GetMatrix().Pop(Render::Matrix::kPROJECT);
	mmc::mRender.GetMatrix().Pop(Render::Matrix::kVIEW);
	mmc::mRender.GetMatrix().Pop(Render::Matrix::kMODEL);
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

const glm::mat4 LightSpot::GetShadowMat() const
{
	return _shadowMat;
}

const Bitmap * LightSpot::GetShadowTex() const
{
	return _shadowTex;
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
		mmc::mRender.GetMatrix().Identity(Render::Matrix::kVIEW);
		mmc::mRender.GetMatrix().Identity(Render::Matrix::kMODEL);
		mmc::mRender.GetMatrix().Identity(Render::Matrix::kPROJECT);
		mmc::mRender.GetMatrix().Mul(Render::Matrix::kVIEW, view);
		mmc::mRender.GetMatrix().Mul(Render::Matrix::kPROJECT, proj);

		_shadowRT->Beg();
		glCullFace(GL_FRONT);
		mmc::mRender.OnRenderCamera(nullptr);
		glCullFace(GL_BACK);
		_shadowRT->End();

		mmc::mRender.GetMatrix().Pop(Render::Matrix::kPROJECT);
		mmc::mRender.GetMatrix().Pop(Render::Matrix::kVIEW);
		mmc::mRender.GetMatrix().Pop(Render::Matrix::kMODEL);
	}
}
