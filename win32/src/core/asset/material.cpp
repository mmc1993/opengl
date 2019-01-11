#include "material.h"
#include "bitmap.h"
#include "../mmc.h"
#include "../render/render.h"

void Material::Bind()
{
	_data.mMesh->Bind();
	_data.mShader->Bind();
	mmc::mRender.Bind(this);
	mmc::mRender.Bind(_data.mMesh);
	mmc::mRender.Bind(_data.mShader);
	_data.mShader->SetUniform("mvp_", mmc::mRender.GetMatrix().GetMVP());
	_data.mShader->SetUniform("mv_", mmc::mRender.GetMatrix().GetMV());
	_data.mShader->SetUniform("texture_", _data.mTexture, 0);
	_data.mShader->SetUniform("normal_", _data.mNormal, 1);
}

void Material::Draw()
{
	mmc::mRender.RenderMesh(_data.mMesh->GetVertexs().size());
}

void Material::Free()
{
	mmc::mRender.Bind((Material *)nullptr);
	mmc::mRender.Bind((Shader *)nullptr);
	mmc::mRender.Bind((Mesh *)nullptr);
	_data.mShader->Free();
	_data.mMesh->Free();
}