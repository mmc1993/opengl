#include "sprite.h"
#include "../mmc.h"
#include "../render/render.h"
#include "../asset/asset_core.h"

void Sprite::OnAdd()
{
}

void Sprite::OnDel()
{
}

void Sprite::OnUpdate(float dt)
{
	Render::Command command;
	command.mCameraID = GetOwner()->GetCameraID();
	command.mCallFn = [this]() {
		mmc::mRender.Bind(_mesh);
		mmc::mRender.Bind(_shader);
		_shader->SetUniform("material_.mDiffuse", _material->mData.mDiffuse, 0);
		_shader->SetUniform("material_.mSpecular", _material->mData.mSpecular,1);
		_shader->SetUniform("material_.mShininess", _material->mData.mShininess);
		glEnable(GL_DEPTH_TEST);
		mmc::mRender.RenderMesh();
		glDisable(GL_DEPTH_TEST);
	};
	mmc::mRender.PostCommand(command);
}
