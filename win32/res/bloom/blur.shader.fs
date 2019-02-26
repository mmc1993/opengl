#version 330 core

uniform struct Material_ {
	sampler2D mTexture0;
} material_;

uniform float is_vertical;

in V_OUT_ {
    vec2 mUV;
} v_out_;

layout(location = 0) out vec4 color_;

void main()
{
	vec3 outcolor = vec3(0);
	
	vec2 uvstep = 1.0f / textureSize(material_.mTexture0, 0);

	if (1.0f == is_vertical)
	{
		//	´¹Ö±
		for (int i = -16; i != 16; ++i)
		{
			vec2 uv = vec2(v_out_.mUV.x, v_out_.mUV.y + i * uvstep.y);
			vec3 color = texture(material_.mTexture0, uv).rgb;
			outcolor += color;
		}
	}
	else
	{
		//	Ë®Æ½
		for (int i = -16; i != 16; ++i)
		{
			vec2 uv = vec2(v_out_.mUV.x + i * uvstep.x, v_out_.mUV.y);
			vec3 color = texture(material_.mTexture0, uv).rgb;
			outcolor  += color;
		}
	}

	color_ = vec4(outcolor / 32.0f, 1);
}