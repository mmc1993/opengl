#pragma once

#include "component.h"
#include "../asset/bitmap.h"
#include "../asset/bitmap_cube.h"
#include "../tools/debug_tool.h"

class RenderBuffer {
public:
    RenderBuffer(int w, int h, int fmt)
    {
        _w = w; _h = h; _format = fmt;
        glGenRenderbuffers(1, &_GLID);
        glBindRenderbuffer(GL_RENDERBUFFER, _GLID);
        glRenderbufferStorage(GL_RENDERBUFFER, fmt, w, h);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        ASSERT_LOG(glGetError() == 0, "Create RBO Error");
    }

    ~RenderBuffer()
    {
        glDeleteRenderbuffers(1, &_GLID);
    }
    
    GLuint GetGLID() const { return _GLID; }
private:
    int _w, _h;
    int _format;
    GLuint _GLID;
};

class RenderTarget : public Component {
public:
    enum BindType {
        kDRAW_READ = GL_FRAMEBUFFER,
        kDRAW = GL_DRAW_FRAMEBUFFER,
        kREAD = GL_READ_FRAMEBUFFER,
        kNONE = GL_NONE,
    };

	enum AttachmentType {
		kCOLOR0 = GL_COLOR_ATTACHMENT0,
		kCOLOR1 = GL_COLOR_ATTACHMENT0 + 1,
		kCOLOR2 = GL_COLOR_ATTACHMENT0 + 2,
		kCOLOR3 = GL_COLOR_ATTACHMENT0 + 3,
		kCOLOR4 = GL_COLOR_ATTACHMENT0 + 4,
		kCOLOR5 = GL_COLOR_ATTACHMENT0 + 5,
		kCOLOR6 = GL_COLOR_ATTACHMENT0 + 6,
		kDEPTH = GL_DEPTH_ATTACHMENT,
		kSTENCIL = GL_DEPTH_STENCIL_ATTACHMENT,
	};

	enum TextureType {
		k2D				= GL_TEXTURE_2D,
		k3D_RIGHT		= GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		k3D_LEFT		= GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1,
		k3D_TOP			= GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2,
		k3D_BOTTOM		= GL_TEXTURE_CUBE_MAP_POSITIVE_X + 3,
		k3D_FRONT		= GL_TEXTURE_CUBE_MAP_POSITIVE_X + 4,
		k3D_BACK		= GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5,
	};

public:
    static RenderBuffer * CreateBuffer(int fmt, int w, int h);
	static RenderTexture2D * CreateTexture2D(const std::uint32_t w, const std::uint32_t h, AttachmentType attachment, int texfmt, int rawfmt, int pixtype);
	static RenderTexture3D * CreateTexture3D(const std::uint32_t w, const std::uint32_t h, AttachmentType attachment, int texfmt, int rawfmt, int pixtype);

	RenderTarget();
	~RenderTarget();
	virtual void OnAdd() override;
	virtual void OnDel() override;
	virtual void OnUpdate(float dt) override;

    void BindAttachment(AttachmentType attachment, RenderBuffer * buffer, BindType bindType = BindType::kNONE);
    void BindAttachment(AttachmentType attachment, TextureType type, RenderTexture2D * texture2D, BindType bindType = BindType::kNONE);
    void BindAttachment(AttachmentType attachment, TextureType type, RenderTexture3D * texture3D, BindType bindType = BindType::kNONE);
	
    void Beg(BindType bindType = BindType::kDRAW_READ);
	void End();

private:
	GLuint _fbo;

    BindType _bindType;
};