#version 330 core

uniform struct Material_ {
	sampler2D mTexture;
} material_;

in V_OUT_ {
    vec2 mUV;
} v_out_;

layout(location = 0) out vec4 color_;
layout(location = 1) out vec4 bloom_;

void main()
{
	vec3 color = texture(material_.mTexture, v_out_.mUV).rgb;

	//	hdr
	color = color / (color + vec3(1));

	color_ = vec4(color, 1.0f);

	//	²É¼¯·º¹â
	float v = (color.r + color.g + color.b) / 3;

	bloom_ = vec4(1, 1, 1, 1);

	//if (v > 0.5f) bloom_ = vec4(color, 1.0f);
	//else bloom_ = vec4(0.0f);
}