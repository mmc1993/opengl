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

uniform struct Material_ {
	sampler2D mNormal0;
	sampler2D mNormal1;
	sampler2D mNormal2;
	sampler2D mNormal3;

	sampler2D mDiffuse0;
	sampler2D mDiffuse1;
	sampler2D mDiffuse2;
	sampler2D mDiffuse3;

	sampler2D mSpecular0;
	sampler2D mSpecular1;
	sampler2D mSpecular2;
	sampler2D mSpecular3;
	float mShininess;
	float mFlipUVX;
	float mFlipUVY;
} material_;

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
    v_out_.mUV.x = abs(material_.mFlipUVX - a_uv_.x);
    v_out_.mUV.y = abs(material_.mFlipUVY - a_uv_.y);
    v_out_.mMPos = (matrix_m_ * aPos).xyz;
    v_out_.mMVPos = (matrix_mv_ * aPos).xyz;
    v_out_.mMVPPos = matrix_mvp_ * aPos;
    v_out_.mNormal = normalize(matrix_n_ * a_n_);
    gl_Position = v_out_.mMVPPos;
}