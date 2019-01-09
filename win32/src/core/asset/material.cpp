#include "material.h"
#include "../mmc.h"
#include "../render/render.h"

void Material::Bind()
{
	_data.mMesh->Bind();
	_data.mShader->Bind();
	_data.mShader->SetUniform("mvp_", mmc::mRender.GetMatrix().GetMVP());
	_data.mShader->SetUniform("mv_", mmc::mRender.GetMatrix().GetMV());
	_data.mShader->SetUniform("texture_", _data.mTexture);
	_data.mShader->SetUniform("normal_", _data.mNormal);
}

void Material::Draw()
{
	mmc::mRender.RenderMesh(_data.mMesh->GetVertexs().size());
}

void Material::Free()
{
	_data.mMesh->Free();
	_data.mShader->Free();
}