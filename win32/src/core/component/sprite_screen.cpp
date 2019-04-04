#include "sprite_screen.h"
#include "../mmc.h"
#include "../render/render.h"

SpriteScreen::SpriteScreen()
	: _flipUVX(0.0f)
	, _flipUVY(0.0f)
	, _shader(nullptr)
	, _meshQuat({
			{ { -1.0f, -1.0f, 0.0f },{ 0, 0 },{ 0, 0, 0 },{ 0, 0, 0 },{ 0, 0, 0 } },
			{ {  1.0f, -1.0f, 0.0f },{ 1, 0 },{ 0, 0, 0 },{ 0, 0, 0 },{ 0, 0, 0 } },
			{ {  1.0f,  1.0f, 0.0f },{ 1, 1 },{ 0, 0, 0 },{ 0, 0, 0 },{ 0, 0, 0 } },
			{ { -1.0f,  1.0f, 0.0f },{ 0, 1 },{ 0, 0, 0 },{ 0, 0, 0 },{ 0, 0, 0 } },
		}, { 0, 1, 2, 0, 2, 3 })
{
}

SpriteScreen::~SpriteScreen()
{
}

void SpriteScreen::OnAdd()
{
}

void SpriteScreen::OnDel()
{
}

void SpriteScreen::OnUpdate(float dt)
{
	Render::Command command;
	command.mCameraFlag = GetOwner()->GetCameraFlag();
	command.mCallFn = [this]() {
		mmc::mRender.Bind(_shader);
		_shader->SetUniform("material_.mFlipUVX", _flipUVX);
		_shader->SetUniform("material_.mFlipUVY", _flipUVY);
		for (auto i = 0; i != _textures.size(); ++i)
		{
			mmc::mRender.BindTexture(SFormat("material_.mTexture{0}", i), _textures.at(i));
		}
		mmc::mRender.RenderIdx(_meshQuat.GetGLID(), _meshQuat.GetIdxCount());
	};
	mmc::mRender.PostCommand(command);
}
