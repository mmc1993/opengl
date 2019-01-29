#version 410 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

uniform vec3 camera_pos_;
uniform vec3 camera_eye_;
uniform mat4 matrix_mvp_;
uniform mat4 matrix_mv_;
uniform mat4 matrix_m_;
uniform mat4 matrix_p_;
uniform mat4 matrix_v_;
uniform mat3 matrix_n_;

in V_OUT_ {
    vec3 mNormal;
    vec3 mMPos;
    vec2 mUV;
} v_out_[];

vec3 GetNormal()
{
	vec3 a = vec3(v_out_[1].mMPos) - vec3(v_out_[0].mMPos);
	vec3 b = vec3(v_out_[2].mMPos) - vec3(v_out_[1].mMPos);
	return normalize(cross(a, b));
}

void main()
{
	vec3 normal = GetNormal();

	gl_Position = matrix_p_ * matrix_v_ * vec4(v_out_[0].mMPos, 1.0f);
	EmitVertex();

	gl_Position = matrix_p_ * matrix_v_ * vec4(v_out_[0].mMPos + normal * 0.2f, 1.0f);
	EmitVertex();

	EndPrimitive();

	/*
	gl_Position = matrix_p_ * matrix_v_ * vec4(v_out_[1].mMPos, 1.0f);
	EmitVertex();

	gl_Position = matrix_p_ * matrix_v_ * vec4(v_out_[1].mMPos + normal * 0.2f, 1.0f);
	EmitVertex();

	EndPrimitive();


	gl_Position = matrix_p_ * matrix_v_ * vec4(v_out_[2].mMPos, 1.0f);
	EmitVertex();

	gl_Position = matrix_p_ * matrix_v_ * vec4(v_out_[2].mMPos + normal * 0.2f, 1.0f);
	EmitVertex();

	EndPrimitive();*/
}