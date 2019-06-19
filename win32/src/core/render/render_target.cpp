#include "render_target.h"

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

void RenderTarget::Bind(BindType bindType, const RenderTarget & rt)
{
    glBindFramebuffer(bindType, rt.GetGLID());
}

void RenderTarget::Bind(BindType bindType)
{
    ASSERT_LOG(glCheckFramebufferStatus(bindType) == GL_FRAMEBUFFER_COMPLETE, "FBO Error");

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
