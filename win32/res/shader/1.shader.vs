#version 410 core

layout(location = 0) in vec3 a_pos_;
layout(location = 1) in vec2 a_uv_;

uniform vec3 camera_pos_;
uniform vec3 camera_eye_;
uniform mat4 mvp_;
uniform mat4 mv_;

out vec2 vuv_;
out vec4 vpos_mv_;
out vec4 vpos_mvp_;

void main()
{
    vuv_ = a_uv_;
    vpos_mv_ = mv_ * vec4(a_pos_, 1.0);
    vpos_mvp_ = mvp_ * vec4(a_pos_, 1.0);
    gl_Position = vpos_mvp_;
}