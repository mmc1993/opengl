#include "sprite.h"
#include "../render/renderer.h"

Sprite::Sprite() : _material(nullptr)
{ }

void Sprite::OnUpdate(float dt)
{
    MaterialCommand command;
    command.mCameraMask = GetOwner()->GetCameraFlag();
    command.mMaterial   = _material;
    command.mSubPass    = 0xffffffff;
    command.mTransform  = Global::Ref().RefRenderer().GetMatrixStack().GetM();
	Global::Ref().RefRenderer().Post(CommandEnum::kMATERIAL, &command);
}
