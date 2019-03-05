#include "render_target.h"
#include "../asset/bitmap.h"
#include "../asset/bitmap_cube.h"
#include "../tools/debug_tool.h"

Bitmap * RenderTarget::Create2DTexture(const std::uint32_t w, const std::uint32_t h, AttachmentType attachment, int texfmt, int glfmt, int gltype)
{
	Bitmap * bitmap = nullptr;
	switch (attachment)
	{
	case kCOLOR0:
	case kCOLOR1:
	case kCOLOR2:
	case kCOLOR3:
	case kCOLOR4:
	case kCOLOR5:
	case kCOLOR6:
		{
			bitmap = new Bitmap(w, h, texfmt, glfmt, gltype, "RenderTarget Color", nullptr);
			bitmap->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			bitmap->SetParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		break;
	case kDEPTH:
		{
			bitmap = new Bitmap(w, h, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, gltype, "RenderTarget Depth", nullptr);
			bitmap->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			bitmap->SetParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		break;
	case kSTENCIL:
		{
			bitmap = new Bitmap(w, h, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, "RenderTarget Stencil", nullptr);
			bitmap->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			bitmap->SetParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
	case kCOLOR0:
	case kCOLOR1:
	case kCOLOR2:
	case kCOLOR3:
	case kCOLOR4:
	case kCOLOR5:
	case kCOLOR6:
		{
			bitmap = new BitmapCube(w, h, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, { }, { });
			bitmap->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			bitmap->SetParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		break;
	case kDEPTH:
		{
			bitmap = new BitmapCube(w, h, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, { }, { });
			bitmap->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			bitmap->SetParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		break;
	case kSTENCIL:
		{
			bitmap = new BitmapCube(w, h, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, { }, { });
			bitmap->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			bitmap->SetParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			bitmap->SetParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
	auto n = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	ASSERT_RET(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
