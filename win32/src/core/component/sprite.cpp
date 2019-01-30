#include "sprite.h"
#include "../mmc.h"
#include "../asset/file.h"
#include "../third/sformat.h"
#include "../render/render.h"
#include "../asset/asset_core.h"

Sprite::Sprite()
	: _shader(nullptr)
	, _showNormal(nullptr)
	, _flipUVX(0)
	, _flipUVY(0)
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
			_shader->SetUniform("material_.mShininess", _materials.at(i).mShininess);
			_shader->SetUniform("material_.mFlipUVX", _flipUVX);
			_shader->SetUniform("material_.mFlipUVY", _flipUVY);
			mmc::mRender.RenderIdx(_meshs.at(i)->GetGLID(), _meshs.at(i)->GetIdxCount());
		}

		if (nullptr != _showNormal)
		{
			mmc::mRender.Bind(_showNormal);
			for (auto i = 0; i != _meshs.size(); ++i)
			{
				mmc::mRender.RenderIdx(_meshs.at(i)->GetGLID(), _meshs.at(i)->GetIdxCount());
			}
		}

		//	关闭混合
		glDisable(GL_BLEND);
		//	关闭深度测试
		glDisable(GL_DEPTH_TEST);
	};
	mmc::mRender.PostCommand(command);
}

void Sprite::ShowNormal(bool isTrue)
{
	if (isTrue && nullptr == _showNormal)
	{
		_showNormal = File::LoadShader("res/geometry/show_normal.shader");
	}
	if (!isTrue && nullptr == _showNormal)
	{
		_showNormal = nullptr;
	}
}
