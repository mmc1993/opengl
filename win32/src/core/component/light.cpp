#include "light.h"
#include "../mmc.h"
#include "../render/render.h"

Light::Light()
{
}

Light::~Light()
{
}

void Light::OnAdd()
{
	mmc::mRender.AddLight(this);
}

void Light::OnDel()
{
	mmc::mRender.DelLight(this);
}

void Light::OnUpdate(float dt)
{
	Render::Command command;
	command.mCameraID = GetOwner()->GetCameraID();
	command.mCallFn = [this]() {
		mmc::mRender.Bind(mMesh);
		mmc::mRender.Bind(mShader);
		glEnable(GL_DEPTH_TEST);
		mmc::mRender.RenderMesh();
		glDisable(GL_DEPTH_TEST);
	};
	mmc::mRender.PostCommand(command);
}
