#version 410 core

layout(location = 0) in vec3 a_pos_;
layout(location = 1) in vec2 a_uv_;
layout(location = 2) in vec3 a_n_;

uniform vec3 camera_pos_;
uniform vec3 camera_eye_;
uniform mat4 matrix_mvp_;
uniform mat4 matrix_mv_;
uniform mat4 matrix_m_;
uniform mat3 matrix_n_;

out V_OUT_ {
    vec3 mNormal;
    vec3 mMPos;
    vec2 mUV;
} v_out_;

void main()
{
	vec4 aPos = vec4(a_pos_, 1);
    v_out_.mUV = a_uv_;
    v_out_.mMPos = vec3(matrix_m_ * vec4(a_pos_, 1));
    v_out_.mNormal = normalize(matrix_n_ * a_n_);
}