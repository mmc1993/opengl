#include "sprite.h"
#include "../mmc.h"
#include "../render/render.h"

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
		mmc::mRender.Bind(_material);
		mmc::mRender.RenderMesh();
	};
	mmc::mRender.PostCommand(command);
}
