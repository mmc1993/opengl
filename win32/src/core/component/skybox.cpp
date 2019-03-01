#include "skybox.h"
#include "../mmc.h"
#include "../render/render.h"

Skybox::Skybox()
	: _vao(0)
	, _vbo(0)
	, _ebo(0)
{
	float vertexs[] = {
		-1.0f,		 1.0f,		-1.0f,
		-1.0f,		-1.0f,		-1.0f,
		 1.0f,		-1.0f,		-1.0f,
		 1.0f,		-1.0f,		-1.0f,
		 1.0f,		 1.0f,		-1.0f,
		-1.0f,		 1.0f,		-1.0f,

		-1.0f,		-1.0f,		 1.0f,
		-1.0f,		-1.0f,		-1.0f,
		-1.0f,		 1.0f,		-1.0f,
		-1.0f,		 1.0f,		-1.0f,
		-1.0f,		 1.0f,		 1.0f,
		-1.0f,		-1.0f,		 1.0f,

		 1.0f,		-1.0f,		-1.0f,
		 1.0f,		-1.0f,		 1.0f,
		 1.0f,		 1.0f,		 1.0f,
		 1.0f,		 1.0f,		 1.0f,
		 1.0f,		 1.0f,		-1.0f,
		 1.0f,		-1.0f,		-1.0f,

		-1.0f,		-1.0f,		 1.0f,
		-1.0f,		 1.0f,		 1.0f,
		 1.0f,		 1.0f,		 1.0f,
		 1.0f,		 1.0f,		 1.0f,
		 1.0f,		-1.0f,		 1.0f,
		-1.0f,		-1.0f,		 1.0f,

		-1.0f,		 1.0f,		-1.0f,
		 1.0f,		 1.0f,		-1.0f,
		 1.0f,		 1.0f,		 1.0f,
		 1.0f,		 1.0f,		 1.0f,
		-1.0f,		 1.0f,		 1.0f,
		-1.0f,		 1.0f,		-1.0f,

		-1.0f,		-1.0f,		-1.0f,
		-1.0f,		-1.0f,		 1.0f,
		 1.0f,		-1.0f,		-1.0f,
		 1.0f,		-1.0f,		-1.0f,
		-1.0f,		-1.0f,		 1.0f,
		 1.0f,		-1.0f,		 1.0f
	};

	int indices[] = {
		0, 1, 2, 3, 4, 5, 
		6, 7, 8, 9, 10, 11, 
		12, 13, 14, 15, 16, 17, 
		18, 19, 20, 21, 22, 23, 
		24, 25, 26, 27, 28, 29, 
		30, 31, 32, 33, 34, 35, 
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
