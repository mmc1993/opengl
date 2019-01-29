#include "sprite_batch.h"
#include "../mmc.h"
#include "../asset/file.h"
#include "../third/sformat.h"
#include "../render/render.h"
#include "../asset/asset_core.h"

SpriteBatch::SpriteBatch(): _count(0)
{
}

SpriteBatch::~SpriteBatch()
{
}

void SpriteBatch::OnAdd()
{
}

void SpriteBatch::OnDel()
{
}

void SpriteBatch::OnUpdate(float dt)
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
			for (auto j = 0; j != _materials.at(i).mNormals.size(); ++j)
			{
				mmc::mRender.BindTexture(SFormat("material_.mNormal{0}", j), _materials.at(i).mNormals.at(j));
			}
			for (auto j = 0; j != _materials.at(i).mDiffuses.size(); ++j)
			{
				mmc::mRender.BindTexture(SFormat("material_.mDiffuse{0}", j), _materials.at(i).mDiffuses.at(j));
			}
			for (auto j = 0; j != _materials.at(i).mSpeculars.size(); ++j)
			{
				mmc::mRender.BindTexture(SFormat("material_.mSpecular{0}", j), _materials.at(i).mSpeculars.at(j));
			}
			for (auto j = 0; j != _materials.at(i).mReflects.size(); ++j)
			{
				mmc::mRender.BindTexture(SFormat("material_.mReflect{0}", j), _materials.at(i).mReflects.at(j));
			}
			_shader->SetUniform("material_.mFlipUVX", _flipUVX);
			_shader->SetUniform("material_.mFlipUVY", _flipUVY);
			_shader->SetUniform("material_.mShininess", _materials.at(i).mShininess);
			mmc::mRender.RenderIdxInst(_meshs.at(i)->GetGLID(), _meshs.at(i)->GetIndices().size(), _count);
		}

		if (nullptr != _showNormal)
		{
			mmc::mRender.Bind(_showNormal);
			for (auto i = 0; i != _meshs.size(); ++i)
			{
				mmc::mRender.Bind(_meshs.at(i));
				mmc::mRender.RenderMesh();
			}
		}

		//	关闭混合
		glDisable(GL_BLEND);
		//	关闭深度测试
		glDisable(GL_DEPTH_TEST);
	};
	mmc::mRender.PostCommand(command);
}
