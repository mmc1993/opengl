#version 410 core

layout(location = 0) in vec3 a_pos_;
layout(location = 1) in vec2 a_uv_;
layout(location = 2) in vec3 a_n_;
layout(location = 3) in vec3 a_t_;
layout(location = 4) in vec3 a_b_;


uniform vec3 camera_pos_;
uniform vec3 camera_eye_;
uniform mat4 matrix_mvp_;
uniform mat4 matrix_mv_;
uniform mat4 matrix_m_;
uniform mat4 matrix_p_;
uniform mat4 matrix_v_;
uniform mat3 matrix_n_;

out V_OUT_ {
    vec3 mNormal;
    vec4 mMVPPos;
    vec3 mMVPos;
    vec3 mMPos;
	mat3 mTBNR;
	mat3 mTBN;
    vec2 mUV;
} v_out_;

void main()
{
	vec4 apos = vec4(a_pos_, 1);
    v_out_.mMPos = vec3(matrix_m_ * apos);
    v_out_.mMVPos = vec3(matrix_mv_ * apos);
	v_out_.mNormal = vec3(matrix_n_ * a_n_);
    v_out_.mMVPPos = vec4(matrix_mvp_ * apos);

	vec3 T = normalize(vec3(matrix_m_ * vec4(a_t_, 0.0f)));
	vec3 B = normalize(vec3(matrix_m_ * vec4(a_b_, 0.0f)));
	vec3 N = normalize(vec3(matrix_m_ * vec4(a_n_, 0.0f)));
	v_out_.mTBN = mat3(T, B, N);
	v_out_.mTBNR = transpose(v_out_.mTBN);

    v_out_.mUV = a_uv_;

    gl_Position = v_out_.mMVPPos;
}