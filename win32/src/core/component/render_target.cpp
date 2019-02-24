#include "render_target.h"
#include "../asset/bitmap.h"
#include "../asset/bitmap_cube.h"
#include "../tools/debug_tool.h"

Bitmap * RenderTarget::Create2DTexture(const std::uint32_t w,
									   const std::uint32_t h, 
									   AttachmentType attachment, 
									   int texFormat, int glFormat, int glType)
{
	Bitmap * bitmap = nullptr;
	switch (attachment)
	{
	case kCOLOR:
		{
			bitmap = new Bitmap(w, h, texFormat, glFormat, glType, "RenderTarget Color", nullptr);
			bitmap->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		break;
	case kDEPTH:
		{
			bitmap = new Bitmap(w, h, GL_DEPTH_COMPONENT, "RenderTarget Depth", nullptr);
			bitmap->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		break;
	case kSTENCIL:
		{
			bitmap = new Bitmap(w, h, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, "RenderTarget Stencil", nullptr);
			bitmap->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		break;
	}
	return bitmap;
}

BitmapCube * RenderTarget::Create3DTexture(const std::uint32_t w, const std::uint32_t h, AttachmentType attachment)
{
	BitmapCube * bitmap = nullptr;
	switch (attachment)
	{
	case kCOLOR:
		{
			bitmap = new BitmapCube(w, h, GL_RGBA,
									{	"RenderTarget Color", "RenderTarget Color", "RenderTarget Color",
										"RenderTarget Color", "RenderTarget Color", "RenderTarget Color" },
									{	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, });
			bitmap->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}
		break;
	case kDEPTH:
		{
			bitmap = new BitmapCube(w, h, GL_DEPTH_COMPONENT,
									{	"RenderTarget Depth", "RenderTarget Depth", "RenderTarget Depth",
										"RenderTarget Depth", "RenderTarget Depth", "RenderTarget Depth" },
									{	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, });
			bitmap->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}
		break;
	case kSTENCIL:
		{
			bitmap = new BitmapCube(w, h, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8,
									{	"RenderTarget Stencil", "RenderTarget Stencil", "RenderTarget Stencil",
										"RenderTarget Stencil", "RenderTarget Stencil", "RenderTarget Stencil" },
									{	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, });
			bitmap->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}
		break;
	}
	return bitmap;
}

RenderTarget::RenderTarget() : _fbo(0)
{ }

RenderTarget::~RenderTarget()
{
	glDeleteFramebuffers(1, &_fbo);
}

void RenderTarget::OnAdd()
{ }

void RenderTarget::OnDel()
{ }

void RenderTarget::OnUpdate(float dt)
{ }

void RenderTarget::BindAttachment(AttachmentType attachment, TextureType type, int texid)
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, type, texid, 0);
}

void RenderTarget::Beg()
{
	if (_fbo == 0)
	{
		glGenFramebuffers(1, &_fbo);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
}

void RenderTarget::End()
{
	ASSERT_RET(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
