#include "sprite.h"
#include "../render/render.h"

Sprite::Sprite() : _material(nullptr)
{ }

void Sprite::OnUpdate(float dt)
{
    MaterialCommand command;
    command.mCameraMask = GetOwner()->GetCameraFlag();
    command.mMaterial   = _material;
    command.mSubPass    = 0xffffffff;
    command.mTransform  = Global::Ref().RefRender().GetMatrixStack().GetM();
	Global::Ref().RefRender().Post(CommandEnum::kMATERIAL, command);
}
