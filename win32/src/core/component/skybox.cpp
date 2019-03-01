#include "skybox.h"
#include "../mmc.h"
#include "../asset/file.h"
#include "../render/render.h"

Skybox::Skybox()
	: _vao(0)
	, _vbo(0)
	, _ebo(0)
	, _bitmapCube(nullptr)
{
	float vertexs[] = {
		-1,  1, -1,
		-1,  1,  1,
		 1,  1,  1,
		 1,  1, -1,
		-1, -1, -1,
		-1, -1,  1,
		 1, -1,  1,
		 1, -1, -1,
	};

	int indices[] = {
		1, 6, 5,
		1, 2, 6,

		0, 4, 7,
		0, 7, 3,

		0, 1, 5,
		0, 5, 4,

		3, 7, 6,
		3, 6, 2,

		0, 3, 2,
		0, 2, 1,

		4, 5, 6,
		4, 6, 7,
	};

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexs), vertexs, GL_STATIC_DRAW);

	glGenBuffers(1, &_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (const void *)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

Skybox::~Skybox()
{
	glDeleteBuffers(1, &_vbo);
	glDeleteBuffers(1, &_ebo);
	glDeleteVertexArrays(1, &_vao);
}

void Skybox::OnAdd()
{
}

void Skybox::OnDel()
{
}

void Skybox::OnUpdate(float dt)
{
	Render::Command command;
	command.mCameraFlag = GetOwner()->GetCameraFlag();
	command.mCallFn = [this]() {
		mmc::mRender.Bind(_shader);
		mmc::mRender.RenderIdx(_vao, 36);
	};
	mmc::mRender.PostCommand(command);
}

void Skybox::BindShader(const std::string & url)
{
	_shader = File::LoadShader(url);
}

void Skybox::BindBitmapCube(const std::string & url)
{
	_bitmapCube = File::LoadBitmapCube(url);
}
