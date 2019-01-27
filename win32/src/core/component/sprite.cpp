#include "sprite.h"
#include "../mmc.h"
#include "../third/sformat.h"
#include "../render/render.h"
#include "../asset/asset_core.h"

Sprite::Sprite()
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
	Render::Command command;
	command.mCameraID = GetOwner()->GetCameraID();
	command.mCallFn = [this]() {
		//	开启混合
		glEnable(GL_BLEND);
		glBlendFunc(_blend.mSrc, _blend.mDst);
		//	开启深度测试
		glEnable(GL_DEPTH_TEST);

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
			_shader->SetUniform("material_.mFlipUVX", _flipUVX);
			_shader->SetUniform("material_.mFlipUVY", _flipUVY);
			_shader->SetUniform("material_.mShininess", _materials.at(i).mShininess);
			mmc::mRender.RenderMesh();
		}
		//	关闭混合
		glDisable(GL_BLEND);
		//	关闭深度测试
		glDisable(GL_DEPTH_TEST);
	};
	mmc::mRender.PostCommand(command);
}
