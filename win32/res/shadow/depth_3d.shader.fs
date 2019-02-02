#version 410 core

uniform samplerCube texture3D_;

in V_OUT_ {
    vec3 mUV;
} v_out_;


out vec4 color_;

void main()
{
	float r = texture(texture3D_, v_out_.mUV).r;
	color_ = vec4(vec3(r), 1.0f);
}