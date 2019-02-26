#version 330 core

uniform struct Material_ {
	sampler2D mTexture0;
	sampler2D mTexture1;
} material_;

in V_OUT_ {
    vec2 mUV;
} v_out_;

layout(location = 0) out vec4 color_;

void main()
{
	vec3 color0 = texture(material_.mTexture0, v_out_.mUV).rgb;
	vec3 color1 = texture(material_.mTexture1, v_out_.mUV).rgb;

	color_ = vec4(color0 + color1, 1.0f);
}