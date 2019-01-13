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
		_material->Draw();
	};
	mmc::mRender.PostCommand(command);
}
