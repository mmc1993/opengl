#include "sprite.h"
#include "../mmc.h"
#include "../asset/file.h"
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
	RenderCommand command;
	command.mCameraFlag			= GetOwner()->GetCameraFlag();
	command.mTransform			= mmc::mRender.GetMatrix().GetM();
	command.mMeshs				= _meshs.data();
	command.mMaterials			= _mates.data();
	command.mMeshNum			= _meshs.size();
	mmc::mRender.PostCommand(_shader, command);
}

void Sprite::BindShader(const std::string & url)
{
	_shader = File::LoadShader(url);
}
