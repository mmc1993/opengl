#include "skybox.h"
#include "../res/file.h"
#include "../render/render.h"

Skybox::Skybox()
	: _shader(nullptr)
	, _meshCube(Mesh::Create({
		{ { -1.0f,  1.0f, -1.0f } }, { { -1.0f,  1.0f,  1.0f } }, { { 1.0f,  1.0f,  1.0f } }, { { 1.0f,  1.0f, -1.0f } },
		{ { -1.0f, -1.0f, -1.0f } }, { { -1.0f, -1.0f,  1.0f } }, { { 1.0f, -1.0f,  1.0f } }, { { 1.0f, -1.0f, -1.0f } }
		},{ 1, 6, 5, 1, 2, 6, 0, 4, 7, 0, 7, 3, 0, 1, 5, 0, 5, 4, 3, 7, 6, 3, 6, 2, 0, 3, 2, 0, 2, 1, 4, 5, 6, 4, 6, 7 }))
{ }

Skybox::~Skybox()
{
	Mesh::DeleteRef(_meshCube);
}

void Skybox::OnAdd()
{
}

void Skybox::OnDel()
{
}

void Skybox::OnUpdate(float dt)
{
    ObjectCommand command;
	command.mCameraFlag		= GetOwner()->GetCameraFlag();
	command.mTransform		= Global::Ref().RefRender().GetMatrixStack().GetM();
	command.mMaterials		= &_material;
	command.mMeshs			= &_meshCube;
	command.mMeshNum		= 1;
	Global::Ref().RefRender().PostCommand(_shader, command);
}

void Skybox::BindShader(Shader * shader)
{
	_shader = shader;
}

void Skybox::BindShader(const std::string & url)
{
	BindShader(File::LoadShader(url));
}

void Skybox::BindTexture(BitmapCube * cube)
{
	_material.mTexCube = cube;
}

void Skybox::BindTexture(const std::string & url)
{
	BindTexture(File::LoadBitmapCube(url));
}