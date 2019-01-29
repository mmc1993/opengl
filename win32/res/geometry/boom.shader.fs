#version 410 core

uniform struct Material_ {
	sampler2D mNormal0;
	sampler2D mNormal1;
	sampler2D mNormal2;
	sampler2D mNormal3;

	sampler2D mDiffuse0;
	sampler2D mDiffuse1;
	sampler2D mDiffuse2;
	sampler2D mDiffuse3;

	sampler2D mSpecular0;
	sampler2D mSpecular1;
	sampler2D mSpecular2;
	sampler2D mSpecular3;
	
	sampler2D mReflect0;
	sampler2D mReflect1;
	sampler2D mReflect2;
	sampler2D mReflect3;
	float mShininess;
} material_;

in G_OUT_ {
	vec2 mUV;
} g_out_;

out vec4 color_;

void main()
{
	color_ = texture(material_.mDiffuse0, g_out_.mUV);
}