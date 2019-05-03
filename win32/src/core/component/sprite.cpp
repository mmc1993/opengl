#include "sprite.h"
#include "../res/file.h"
#include "../render/render.h"

Sprite::Sprite()
	: _shader(nullptr)
{
}

void Sprite::OnAdd()
{
}

void Sprite::OnDel()
{
}

void Sprite::OnUpdate(float dt)
{
    ObjectCommand command;
	command.mCameraFlag			= GetOwner()->GetCameraFlag();
	command.mTransform			= Global::Ref().RefRender().GetMatrixStack().GetM();
	command.mMeshs				= _meshs.data();
	command.mMaterials			= _mates.data();
	command.mMeshNum			= static_cast<uint>(_meshs.size());
	Global::Ref().RefRender().PostCommand(_shader, command);
}

void Sprite::BindShader(const std::string & url)
{
	_shader = File::LoadShader(url);
}
