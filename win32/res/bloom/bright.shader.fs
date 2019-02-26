#version 330 core

uniform struct Material_ {
	sampler2D mTexture0;
} material_;

in V_OUT_ {
    vec2 mUV;
} v_out_;

layout(location = 0) out vec4 color_;

void main()
{
	vec3 color = texture(material_.mTexture0, v_out_.mUV).rgb;
	float v = (color.r + color.g + color.b) / 3;
	if (v > 0.5f) color_ = vec4(color, 1.0f);
	else color_ = vec4(0.0f);
}