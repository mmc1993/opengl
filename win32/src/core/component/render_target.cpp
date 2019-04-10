#include "render_target.h"
#include "../asset/bitmap.h"
#include "../asset/bitmap_cube.h"
#include "../tools/debug_tool.h"

RenderBuffer * RenderTarget::CreateBuffer(int fmt, int w, int h)
{
    return nullptr;
}

RenderTexture2D * RenderTarget::CreateTexture2D(const std::uint32_t w, const std::uint32_t h, AttachmentType attachment, int texfmt, int rawfmt, int pixtype)
{
    RenderTexture2D * texture2D = nullptr;
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
			texture2D = new RenderTexture2D(w, h, texfmt, rawfmt, pixtype, "RenderTarget Color", nullptr);
			texture2D->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			texture2D->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			texture2D->SetParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			texture2D->SetParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		break;
	case kDEPTH:
		{
			texture2D = new RenderTexture2D(w, h, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, pixtype, "RenderTarget Depth", nullptr);
			texture2D->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			texture2D->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			texture2D->SetParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			texture2D->SetParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		break;
	case kSTENCIL:
		{
			texture2D = new RenderTexture2D(w, h, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, "RenderTarget Stencil", nullptr);
			texture2D->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			texture2D->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			texture2D->SetParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			texture2D->SetParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		break;
	}
	return texture2D;
}

RenderTexture3D * RenderTarget::CreateTexture3D(const std::uint32_t w, const std::uint32_t h, AttachmentType attachment, int texfmt, int rawfmt, int pixtype)
{
    RenderTexture3D * texture3D = nullptr;
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
			texture3D = new RenderTexture3D(w, h, texfmt, rawfmt, pixtype);
			texture3D->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			texture3D->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			texture3D->SetParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			texture3D->SetParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			texture3D->SetParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		break;
	case kDEPTH:
		{
			texture3D = new RenderTexture3D(w, h, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, pixtype);
			texture3D->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			texture3D->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			texture3D->SetParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			texture3D->SetParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			texture3D->SetParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		break;
	case kSTENCIL:
		{
			texture3D = new RenderTexture3D(w, h, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
			texture3D->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			texture3D->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			texture3D->SetParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			texture3D->SetParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			texture3D->SetParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		break;
	}
	return texture3D;
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

void RenderTarget::BindAttachment(AttachmentType attachment, RenderBuffer * buffer, BindType bindType)
{
    if (bindType == BindType::kNONE) { bindType = _bindType; }

    glFramebufferRenderbuffer(bindType, attachment, GL_RENDERBUFFER, buffer->GetGLID());
}

void RenderTarget::BindAttachment(AttachmentType attachment, TextureType type, RenderTexture2D * texture2D, BindType bindType)
{
    if (bindType == BindType::kNONE) { bindType = _bindType; }

    glFramebufferTexture2D(bindType, attachment, type, texture2D->GetGLID(), 0);
}

void RenderTarget::BindAttachment(AttachmentType attachment, TextureType type, RenderTexture3D * texture3D, BindType bindType)
{
    if (bindType == BindType::kNONE) { bindType = _bindType; }

    glFramebufferTexture2D(bindType, attachment, type, texture3D->GetGLID(), 0);
}

void RenderTarget::Beg(BindType bindType)
{
	if (_fbo == 0)
	{
		glGenFramebuffers(1, &_fbo);
	}
	glBindFramebuffer(_bindType, _fbo);
    
    _bindType = bindType;
}

void RenderTarget::End()
{
	ASSERT_RET(glCheckFramebufferStatus(_bindType) == GL_FRAMEBUFFER_COMPLETE);

	glBindFramebuffer(_bindType, 0);
}
