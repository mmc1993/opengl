#version 410 core

layout(location = 0) in vec3 a_pos_;

uniform vec3 camera_pos_;
uniform vec3 camera_eye_;
uniform mat4 matrix_mvp_;
uniform mat4 matrix_mv_;
uniform mat4 matrix_m_;
uniform mat4 matrix_p_;
uniform mat4 matrix_v_;
uniform mat3 matrix_n_;


out V_OUT_ {
	vec3 mAPos;
} v_out_;

void main()
{
	v_out_.mAPos = a_pos_;
	mat4 view = mat4(mat3(matrix_v_));
    gl_Position = matrix_p_ * view * vec4(a_pos_, 1.0f);
}