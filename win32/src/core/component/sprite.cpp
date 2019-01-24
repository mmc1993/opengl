#include "sprite.h"
#include "../mmc.h"
#include "../third/sformat.h"
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
		mmc::mRender.Bind(_shader);
		for (auto i = 0; i != _meshs.size(); ++i)
		{
			mmc::mRender.Bind(_meshs.at(i));
			auto textureNum = 0;
			for (auto j = 0; j != _materials.at(i).mNormals.size(); ++j)
			{
				_shader->SetUniform(SFormat("material_.mNormal{0}", j), _materials.at(i).mNormals.at(j), textureNum++);
			}
			for (auto j = 0; j != _materials.at(i).mDiffuses.size(); ++j)
			{
				_shader->SetUniform(SFormat("material_.mDiffuse{0}", j), _materials.at(i).mDiffuses.at(j), textureNum++);
			}
			for (auto j = 0; j != _materials.at(i).mSpeculars.size(); ++j)
			{
				_shader->SetUniform(SFormat("material_.mSpecular{0}", j), _materials.at(i).mSpeculars.at(j), textureNum++);
			}
			_shader->SetUniform("material_.mShininess", _materials.at(i).mShininess);
			mmc::mRender.RenderMesh();
		}
	};
	mmc::mRender.PostCommand(command);
}
