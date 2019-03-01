#include "sprite_batch.h"
#include "../mmc.h"
#include "../asset/file.h"
#include "../third/sformat.h"
#include "../render/render.h"

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
	command.mCameraIdx = GetOwner()->GetCameraIdx();
	command.mCallFn = [this]() {
		//	开启混合
		glEnable(GL_BLEND);
		glBlendFunc(_blend.mSrc, _blend.mDst);
		//	开启深度测试
		glEnable(GL_DEPTH_TEST);

		mmc::mRender.Bind(_shader);
		for (auto i = 0; i != _meshs.size(); ++i)
		{
			for (auto j = 0; j != _materials.at(i).mDiffuses.size(); ++j)
			{
				mmc::mRender.BindTexture(SFormat("material_.mDiffuse{0}", j), _materials.at(i).mDiffuses.at(j));
			}
			if (_materials.at(i).mParallax.GetBitmap() != nullptr)
			{
				mmc::mRender.BindTexture("material_.mParallax", _materials.at(i).mParallax);
			}
			if (_materials.at(i).mSpecular.GetBitmap() != nullptr)
			{
				mmc::mRender.BindTexture("material_.mSpecular", _materials.at(i).mSpecular);
			}
			if (_materials.at(i).mReflect.GetBitmap() != nullptr)
			{
				mmc::mRender.BindTexture("material_.mReflect", _materials.at(i).mReflect);
			}
			if (_materials.at(i).mNormal.GetBitmap() != nullptr)
			{
				mmc::mRender.BindTexture("material_.mNormal", _materials.at(i).mNormal);
			}
			_shader->SetUniform("material_.mFlipUVX", _flipUVX);
			_shader->SetUniform("material_.mFlipUVY", _flipUVY);
			_shader->SetUniform("material_.mShininess", _materials.at(i).mShinines);
			mmc::mRender.RenderIdxInst(_meshs.at(i)->GetGLID(), _meshs.at(i)->GetIndices().size(), _count);
		}

		//	关闭混合
		glDisable(GL_BLEND);
		//	关闭深度测试
		glDisable(GL_DEPTH_TEST);
	};
	mmc::mRender.PostCommand(command);
}
