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
		mmc::mRender.Bind(_material);
		_shader->SetUniform("texture0_", _texture0, 0);
		_shader->SetUniform("texture1_", _texture1, 1);
		glEnable(GL_DEPTH_TEST);
		mmc::mRender.RenderMesh();
		glDisable(GL_DEPTH_TEST);
	};
	mmc::mRender.PostCommand(command);
}

void Sprite::SetTexture0(const std::string & url)
{
	_texture0.SetTexture(mmc::mAssetCore.Get<Bitmap>(url));
}

void Sprite::SetTexture1(const std::string & url)
{
	_texture1.SetTexture(mmc::mAssetCore.Get<Bitmap>(url));
}