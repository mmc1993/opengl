#pragma once

#include "component.h"
#include "../asset/bitmap.h"
#include "../asset/bitmap_cube.h"

class Bitmap;
class BitmapCube;

class RenderTarget : public Component {
public:
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
	static Bitmap * Create2DTexture(const std::uint32_t w, const std::uint32_t h, AttachmentType attachment, 
									int texfmt = GL_RGBA, int glfmt = GL_RGBA, int gltype = GL_UNSIGNED_BYTE);

	static BitmapCube * Create3DTexture(const std::uint32_t w, const std::uint32_t h, AttachmentType attachment);

	RenderTarget();
	~RenderTarget();
	virtual void OnAdd() override;
	virtual void OnDel() override;
	virtual void OnUpdate(float dt) override;

	void BindAttachment(AttachmentType attachment, TextureType type, int texid);
	void Beg();
	void End();

private:
	GLuint _fbo;
};