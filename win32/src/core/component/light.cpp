#include "light.h"
#include "transform.h"
#include "render_target.h"
#include "../mmc.h"
#include "../asset/file.h"
#include "../asset/shader.h"
#include "../render/render.h"
#include "../asset/asset_core.h"

Light::Light(LightType type): _type(type), _shadowRT(nullptr)
{
	float vertexs[] = {
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
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
	delete _shadowRT;
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
		command.mCameraIdx = GetOwner()->GetCameraIdx();
		command.mCallFn = [this]() {
			glEnable(GL_DEPTH_TEST);
			mmc::mRender.Bind(_shader);
			mmc::mRender.RenderIdx(_vao, 36);
			glDisable(GL_DEPTH_TEST);
		};
		mmc::mRender.PostCommand(command);
	}
}

void LightDirect::OpenShadow(std::uint32_t depthW, std::uint32_t depthH,
								 float orthoXMin, float orthoXMax, 
								 float orthoYMin, float orthoYMax, 
								 float orthoZMin, float orthoZMax,
								 const glm::vec3 &up)
{
	_up = up; _depthW = depthW; _depthH = depthH;
	_orthoX.x = orthoXMin; _orthoX.y = orthoXMax;
	_orthoY.x = orthoYMin; _orthoY.y = orthoYMax;
	_orthoZ.x = orthoZMin; _orthoZ.y = orthoZMax;
	delete _shadowRT; _shadowRT = new RenderTarget(depthW, depthH, GL_DEPTH_BUFFER_BIT);
}

void LightDirect::HideShadow()
{
	delete _shadowRT; _shadowRT = nullptr;
}

RenderTarget * LightDirect::DrawShadow(bool onlyGet)
{
	if (onlyGet)
	{
		return _shadowRT;
	}
	if (_shadowRT != nullptr)
	{
		auto project = glm::ortho(_orthoX.x, _orthoX.y,
								  _orthoY.x, _orthoY.y,
								  _orthoZ.x, _orthoZ.y);
		auto world = GetOwner()->GetTransform()->GetWorldPosition();
		auto view = glm::lookAt(world, world + mNormal, _up);

		_matrixVP = project * view;

		glViewport(0, 0, _depthW, _depthH);
		mmc::mRender.GetMatrix().Identity(Render::Matrix::kMODEL);
		mmc::mRender.GetMatrix().Identity(Render::Matrix::kVIEW);
		mmc::mRender.GetMatrix().Identity(Render::Matrix::kPROJECT);
		mmc::mRender.GetMatrix().Mul(Render::Matrix::kVIEW, view);
		mmc::mRender.GetMatrix().Mul(Render::Matrix::kPROJECT, project);

		_shadowRT->Beg();
		mmc::mRoot.Update(0);
		glCullFace(GL_FRONT);
		mmc::mRender.OnRenderCamera(nullptr);
		glCullFace(GL_BACK);
		_shadowRT->End();

		mmc::mRender.GetMatrix().Pop(Render::Matrix::kPROJECT);
		mmc::mRender.GetMatrix().Pop(Render::Matrix::kVIEW);
		mmc::mRender.GetMatrix().Pop(Render::Matrix::kMODEL);
	}
	return _shadowRT;
}
