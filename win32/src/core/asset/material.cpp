#include "material.h"
#include "bitmap.h"
#include "../mmc.h"
#include "../render/render.h"

void Material::Draw()
{
	mmc::mRender.Bind(this);
	mmc::mRender.Bind(_data.mMesh);
	mmc::mRender.Bind(_data.mShader);

	_data.mMesh->Bind();
	_data.mShader->Bind();
	_data.mShader->SetUniform("texture_", _data.mTexture, 0);
	_data.mShader->SetUniform("normal_", _data.mNormal, 1);

	mmc::mRender.RenderMesh(_data.mMesh->GetVertexs().size());

	_data.mShader->Free();
	_data.mMesh->Free();
}
