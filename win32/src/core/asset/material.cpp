#include "material.h"
#include "bitmap.h"
#include "../mmc.h"
#include "../render/render.h"

void Material::Bind()
{
	_data.mMesh->Bind();
	_data.mShader->Bind();
	_data.mShader->SetUniform("mvp_", mmc::mRender.GetMatrix().GetMVP());
	_data.mShader->SetUniform("mv_", mmc::mRender.GetMatrix().GetMV());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _data.mTexture.GetBitmap()->GetGLID());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _data.mNormal.GetBitmap()->GetGLID());
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