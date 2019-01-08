//	vertex
#version 330 core
uniform mat4 mvp_;
uniform mat4 mv_;
uniform mat4 m_;

layout (localtion = 0) in vec3 pos_;
layout (localtion = 1) in vec2 uv_;

out vec3 vpos_;
out vec2 vuv_;

void main()
{
	gl_Position = mpv_ * vec4(pos_, 1.0);
	vpos_ = gl_Position;
	vuv_ = uv_;
}


//	frag
#version 330 core
uniform mat4 mvp_;
uniform mat4 mv_;
uniform mat4 m_;
uniform sampler2D normal_;
uniform sampler2D texture_;

in vec3 vpos_;
in vec2 vuv_;

out vec4 color_;

void main()
{
	color_ = texture(texture_, vuv_);
}


