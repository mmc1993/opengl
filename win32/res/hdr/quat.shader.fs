#version 410 core

uniform struct Material_ {
	sampler2D mTexture;
} material_;

in V_OUT_ {
    vec2 mUV;
} v_out_;

out vec4 color_;

void main()
{
	vec3 color = texture(material_.mTexture, v_out_.mUV).rgb;

	color = color / (color + vec3(1));

	color_ = vec4(color, 1.0f);
}