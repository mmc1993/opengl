#version 410 core

layout(location = 0) in vec3 a_pos_;
layout(location = 1) in vec2 a_uv_;
layout(location = 2) in vec3 a_n_;

uniform vec3 camera_pos_;
uniform vec3 camera_eye_;
uniform mat4 mvp_;
uniform mat4 mv_;
uniform mat4 m_;

out V_OUT_ {
    vec3 mNormal;
    vec4 mMVPPos;
    vec3 mMVPos;
    vec3 mMPos;
    vec2 mUV;
} v_out_;

void main()
{
	vec4 aPos = vec4(a_pos_, 1);
    v_out_.mUV = a_uv_;
    v_out_.mMPos = (m_ * aPos).xyz;
    v_out_.mMVPos = (mv_ * aPos).xyz;
    v_out_.mMVPPos = mvp_ * aPos;
    v_out_.mNormal = a_n_;
    gl_Position = v_out_.mMVPPos;
}