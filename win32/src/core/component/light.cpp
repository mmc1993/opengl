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

	if (!mmc::mAssetCore.IsReg("SHADER_LIGHT_"))
	{
		auto vs = R"(
			#version 410 core

			layout(location = 0) in vec3 a_pos_;

			uniform mat4 matrix_mvp_;

			void main()
			{
				gl_Position = matrix_mvp_ * vec4(a_pos_, 1.0);
			})";

		auto fs = R"(
			#version 410 core

			out vec4 color_;

			void main()
			{
				color_ = vec4(1, 1, 1, 1);
			})";
		mmc::mAssetCore.Reg("SHADER_LIGHT_", new Shader(vs, fs));
	}
	_shader = mmc::mAssetCore.Get<Shader>("SHADER_LIGHT_");
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
		mmc::mRender.GetMatrix().Identity(RenderMatrix::kMODEL);
		mmc::mRender.GetMatrix().Identity(RenderMatrix::kPROJECT);
		mmc::mRender.GetMatrix().Mul(RenderMatrix::kVIEW, view);
		mmc::mRender.GetMatrix().Mul(RenderMatrix::kPROJECT, proj);

		_shadowRT->Beg();
		glCullFace(GL_FRONT);
		mmc::mRender.OnRenderCamera(nullptr);
		glCullFace(GL_BACK);
		_shadowRT->End();

		mmc::mRender.GetMatrix().Pop(RenderMatrix::kPROJECT);
		mmc::mRender.GetMatrix().Pop(RenderMatrix::kVIEW);
		mmc::mRender.GetMatrix().Pop(RenderMatrix::kMODEL);
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
	mmc::mRender.GetMatrix().Identity(RenderMatrix::kMODEL);
	mmc::mRender.GetMatrix().Identity(RenderMatrix::kPROJECT);
	mmc::mRender.GetMatrix().Mul(RenderMatrix::kVIEW, view);
	mmc::mRender.GetMatrix().Mul(RenderMatrix::kPROJECT, _proj);

	_shadowRT->Beg();
	_shadowRT->BindAttachment(RenderTarget::AttachmentType::kDEPTH, 
							  (RenderTarget::TextureType)idx,
							  _shadowTex->GetGLID());
	glCullFace(GL_FRONT);
	mmc::mRender.OnRenderCamera(nullptr);
	glCullFace(GL_BACK);
	_shadowRT->End();

	mmc::mRender.GetMatrix().Pop(RenderMatrix::kPROJECT);
	mmc::mRender.GetMatrix().Pop(RenderMatrix::kVIEW);
	mmc::mRender.GetMatrix().Pop(RenderMatrix::kMODEL);
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
		mmc::mRender.GetMatrix().Identity(RenderMatrix::kMODEL);
		mmc::mRender.GetMatrix().Identity(RenderMatrix::kPROJECT);
		mmc::mRender.GetMatrix().Mul(RenderMatrix::kVIEW, view);
		mmc::mRender.GetMatrix().Mul(RenderMatrix::kPROJECT, proj);

		_shadowRT->Beg();
		glCullFace(GL_FRONT);
		mmc::mRender.OnRenderCamera(nullptr);
		glCullFace(GL_BACK);
		_shadowRT->End();

		mmc::mRender.GetMatrix().Pop(RenderMatrix::kPROJECT);
		mmc::mRender.GetMatrix().Pop(RenderMatrix::kVIEW);
		mmc::mRender.GetMatrix().Pop(RenderMatrix::kMODEL);
	}
}
