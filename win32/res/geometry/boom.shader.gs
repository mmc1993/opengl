#version 410 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

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
    vec4 mMVPPos;
    vec3 mMVPos;
    vec3 mMPos;
    vec2 mUV;
} v_out_[];

out G_OUT_ {
	vec2 mUV;
} g_out_;

vec3 GetNormal()
{
	vec3 a = vec3(v_out_[1].mMPos) - vec3(v_out_[0].mMPos);
	vec3 b = vec3(v_out_[2].mMPos) - vec3(v_out_[1].mMPos);
	return normalize(cross(a, b));
}

void main()
{
	vec3 normal = GetNormal() * 0.0f;

    g_out_.mUV = v_out_[0].mUV;
	gl_Position = matrix_p_ * matrix_v_ * vec4(v_out_[0].mMPos + normal, 1.0f);
	EmitVertex();

    g_out_.mUV = v_out_[1].mUV;
	gl_Position = matrix_p_ * matrix_v_ * vec4(v_out_[1].mMPos + normal, 1.0f);
	EmitVertex();

    g_out_.mUV = v_out_[2].mUV;
	gl_Position = matrix_p_ * matrix_v_ * vec4(v_out_[2].mMPos + normal, 1.0f);
	EmitVertex();

	EndPrimitive();
}