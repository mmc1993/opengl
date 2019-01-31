#include "render_target.h"
#include "../tools/debug_tool.h"

RenderTarget::RenderTarget(std::uint32_t w, std::uint32_t h)
	: _w(w)
	, _h(h)
	, _fbo(0)
	, _color(nullptr)
	, _depth(nullptr)
	, _bit(GL_COLOR_BUFFER_BIT | 
		   GL_DEPTH_BUFFER_BIT | 
		   GL_STENCIL_BUFFER_BIT)
{ }

RenderTarget::RenderTarget(std::uint32_t w, std::uint32_t h, GLuint bit)
	: _w(w)
	, _h(h)
	, _fbo(0)
	, _bit(bit)
	, _color(nullptr)
	, _depth(nullptr)
{
}

RenderTarget::~RenderTarget()
{
	delete _color;
	delete _depth;
	glDeleteFramebuffers(1, &_fbo);
}

void RenderTarget::OnAdd()
{
}

void RenderTarget::OnDel()
{
}

void RenderTarget::OnUpdate(float dt)
{
}

void RenderTarget::Beg()
{
	if (_fbo == 0)
	{
		glGenFramebuffers(1, &_fbo);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

	if ((_bit & GL_COLOR_BUFFER_BIT) != 0 && _color == nullptr)
	{
		_color = new Bitmap(_w, _h, GL_RGBA, "RenderTarget Color", nullptr);
		_color->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		_color->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _color->GetGLID(), 0);
	}

	if ((_bit & GL_STENCIL_BUFFER_BIT) != 0 && _depth == nullptr)
	{
		_depth = new Bitmap(_w, _h, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, "RenderTarget Stencil", nullptr);
		_depth->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		_depth->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, _depth->GetGLID(), 0);
	}
	else if ((_bit & GL_DEPTH_BUFFER_BIT) != 0 && _depth == nullptr)
	{
		_depth = new Bitmap(_w, _h, GL_DEPTH_COMPONENT, "RenderTarget Depth", nullptr);
		_depth->SetParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		_depth->SetParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depth->GetGLID(), 0);
	}

	if (_bit == GL_COLOR_BUFFER_BIT)
	{
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	ASSERT_RET(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

void RenderTarget::End()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::uint32_t RenderTarget::GetW() const
{
	return _w;
}

std::uint32_t RenderTarget::GetH() const
{
	return _h;
}

bool RenderTarget::IsColorEmpty() const
{
	return nullptr == _color;
}

bool RenderTarget::IsDepthEmpty() const
{
	return nullptr == _depth;
}

Texture RenderTarget::GetColorTex(bool free)
{
	auto texture = Texture(_color);
	if (free) { _color = nullptr; }
	return texture;
}

Texture RenderTarget::GetDepthTex(bool free)
{
	auto texture = Texture(_depth);
	if (free) { _depth = nullptr; }
	return texture;
}
