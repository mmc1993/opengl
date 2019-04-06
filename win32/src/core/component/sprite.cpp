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
	Render::Command command;
	command.mCameraFlag = GetOwner()->GetCameraFlag();
	command.mCallFn = [this]() {
		for (auto i = 0; i != _meshs.size(); ++i)
		{
			mmc::mRender.Bind(_shader);

			for (auto j = 0; j != _mates.at(i).mDiffuses.size(); ++j)
			{
				mmc::mRender.BindTexture(SFormat("material_.mDiffuse{0}", j), _mates.at(i).mDiffuses.at(j));
			}
			if (_mates.at(i).mHeight.GetBitmap() != nullptr)
			{
				mmc::mRender.BindTexture("material_.mParallax", _mates.at(i).mHeight);
			}
			if (_mates.at(i).mSpecular.GetBitmap() != nullptr)
			{
				mmc::mRender.BindTexture("material_.mSpecular", _mates.at(i).mSpecular);
			}
			if (_mates.at(i).mReflect.GetBitmap() != nullptr)
			{
				mmc::mRender.BindTexture("material_.mReflect", _mates.at(i).mReflect);
			}
			if (_mates.at(i).mNormal.GetBitmap() != nullptr)
			{
				mmc::mRender.BindTexture("material_.mNormal", _mates.at(i).mNormal);
			}
			_shader->SetUniform("material_.mShininess", _mates.at(i).mShininess);
			mmc::mRender.RenderIdx(_meshs.at(i)->GetGLID(), _meshs.at(i)->GetIdxCount());
		}
	};
	mmc::mRender.PostCommand(command);
}

void Sprite::BindShader(const std::string & url)
{
	_shader = File::LoadShader(url);
}
