#version 410 core

uniform sampler2D texture0_;
uniform sampler2D texture1_;
uniform vec3 camera_pos_;
uniform vec3 camera_eye_;
uniform mat4 mvp_;
uniform mat4 mv_;

in vec2 vuv_;
in vec4 vpos_mv_;
in vec4 vpos_mvp_;

out vec4 color_;

void main()
{
	color_ = vec4(1, 1, 1, 1);
}