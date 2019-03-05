#version 410 core

uniform vec3 camera_pos_;
uniform vec3 camera_eye_;
uniform samplerCube skybox_;

in V_OUT_ {
    vec3 mAPos;
} v_out_;

out vec4 color_;

void main()
{
	float r = texture(skybox_, v_out_.mAPos).r;
	color_ = vec4(r, r, r, 1);
}