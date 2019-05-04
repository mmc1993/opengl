#include "render_target.h"
#include "../res/bitmap.h"
#include "../res/bitmap_cube.h"
#include "../tools/debug_tool.h"

RenderBuffer * RenderTarget::CreateBuffer(const std::uint32_t w, const std::uint32_t h, AttachmentType attachment, int fmt)
{
    return attachment == kCOLOR0
        || attachment == kCOLOR1
        || attachment == kCOLOR2
        || attachment == kCOLOR3
        || attachment == kCOLOR4
        || attachment == kCOLOR5
        || attachment == kCOLOR6
        || attachment == kDEPTH
        ? new RenderBuffer(w, h, fmt)
        : new RenderBuffer(w, h, GL_DEPTH24_STENCIL8);
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

void RenderTarget::Bind(BindType bindType, const RenderTarget & rt)
{
    glBindFramebuffer(bindType, rt.GetGLID());
}

void RenderTarget::Bind(BindType bindType)
{
    ASSERT_RET(glCheckFramebufferStatus(bindType) == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(bindType, 0);
}

void RenderTarget::BindAttachment(BindType bindType, AttachmentType attachment, uint buffer)
{
    glFramebufferRenderbuffer(bindType, attachment, GL_RENDERBUFFER, buffer);
}

void RenderTarget::BindAttachment(BindType bindType, AttachmentType attachment, TextureType type, uint texture)
{
    glFramebufferTexture2D(bindType, attachment, type, texture, 0);
}

void RenderTarget::BindAttachment(BindType bindType, AttachmentType attachment, TextureType type, const RenderTexture2D * texture)
{
    BindAttachment(bindType, attachment, type, texture->GetGLID());
}

void RenderTarget::BindAttachment(BindType bindType, AttachmentType attachment, TextureType type, const RenderTexture3D * texture)
{
    BindAttachment(bindType, attachment, type, texture->GetGLID());
}

RenderTarget::RenderTarget() : _fbo(0)
{ }

RenderTarget::~RenderTarget()
{
	glDeleteFramebuffers(1, &_fbo);
}

void RenderTarget::Start(BindType bindType)
{
    _bindType = bindType;

    if (_fbo == 0)
    {
        glGenFramebuffers(1, &_fbo);
    }

    RenderTarget::Bind(_bindType, *this);
}

void RenderTarget::Ended()
{
    RenderTarget::Bind(_bindType);
}

uint RenderTarget::GetGLID() const
{
    return _fbo;
}

void RenderTarget::BindAttachment(AttachmentType attachment, uint buffer)
{
    RenderTarget::BindAttachment(_bindType, attachment, buffer);
}

void RenderTarget::BindAttachment(AttachmentType attachment, TextureType type, uint texture)
{
    RenderTarget::BindAttachment(_bindType, attachment, type, texture);
}

void RenderTarget::BindAttachment(AttachmentType attachment, TextureType type, RenderTexture2D * texture2D)
{
    RenderTarget::BindAttachment(_bindType, attachment, type, texture2D);
}

void RenderTarget::BindAttachment(AttachmentType attachment, TextureType type, RenderTexture3D * texture3D)
{
    RenderTarget::BindAttachment(_bindType, attachment, type, texture3D);
}
