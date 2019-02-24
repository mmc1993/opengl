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
	command.mCameraIdx = GetOwner()->GetCameraIdx();
	command.mCallFn = [this]() {
		mmc::mRender.Bind(_shader);
		_shader->SetUniform("material_.mFlipUVX", _flipUVX);
		_shader->SetUniform("material_.mFlipUVY", _flipUVY);
		mmc::mRender.BindTexture("material_.mTexture", _texture);
		mmc::mRender.RenderIdx(_meshQuat.GetGLID(), _meshQuat.GetIdxCount());
	};
	mmc::mRender.PostCommand(command);
}
