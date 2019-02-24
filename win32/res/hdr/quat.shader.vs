#version 410 core

layout(location = 0) in vec3 a_pos_;
layout(location = 1) in vec2 a_uv_;

out V_OUT_ {
    vec2 mUV;
} v_out_;

void main()
{
    v_out_.mUV = a_uv_;

    gl_Position = vec4(a_pos_, 1);
}