#include "sprite_screen.h"
#include "../asset/file.h"
#include "../render/render.h"

SpriteScreen::SpriteScreen()
	: _shader(nullptr)
	, _meshQuat(RenderMesh::CreateVT({
			{ { -1.0f, -1.0f, 0.0f }, 0, 0 },
			{ {  1.0f, -1.0f, 0.0f }, 1, 0 },
			{ {  1.0f,  1.0f, 0.0f }, 1, 1 },
			{ { -1.0f,  1.0f, 0.0f }, 0, 1 },
		}, { 0, 1, 2, 0, 2, 3 }))
{
}

SpriteScreen::~SpriteScreen()
{
	RenderMesh::Delete(_meshQuat);
}

void SpriteScreen::OnAdd()
{
}

void SpriteScreen::OnDel()
{
}

void SpriteScreen::OnUpdate(float dt)
{
	RenderCommand command;
	command.mCameraFlag		= GetOwner()->GetCameraFlag();
	command.mTransform		= Global::Ref().RefRender().GetMatrix().GetM();
	command.mMaterials		= &_material;
	command.mMeshs			= &_meshQuat;
	command.mMeshNum		= 1;
	Global::Ref().RefRender().PostCommand(_shader, command);
}

void SpriteScreen::BindShader(Shader * shader)
{
	_shader = shader;
}

void SpriteScreen::BindShader(const std::string & url)
{
	BindShader(File::LoadShader(url));
}

void SpriteScreen::ClearBitmap()
{
	_material.mDiffuses.clear();
}

void SpriteScreen::BindBitmap(Bitmap * bitmap)
{
	_material.mDiffuses.push_back(bitmap);
}

void SpriteScreen::BindBitmap(const std::string & url)
{
	BindBitmap(File::LoadBitmap(url));
}
