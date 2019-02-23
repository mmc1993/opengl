#version 410 core

uniform vec3 camera_pos_;
uniform vec3 camera_eye_;

struct LightDirect_ {
	vec3 mNormal;
	
	vec3 mAmbient;
	vec3 mDiffuse;
	vec3 mSpecular;
};

struct LightPoint_ {
	vec3 mPosition;
	float mK0, mK1, mK2;
	
	vec3 mAmbient;
	vec3 mDiffuse;
	vec3 mSpecular;
};

struct LightSpot_ {
	vec3 mNormal;
	vec3 mPosition;
	float mK0, mK1, mK2;
	float mOutCone, mInCone;

	vec3 mAmbient;
	vec3 mDiffuse;
	vec3 mSpecular;
};

uniform struct Light_ {
	int mDirectNum;
	int mPointNum;
	int mSpotNum;
	LightDirect_ mDirects[4];
	LightPoint_ mPoints[8];
	LightSpot_ mSpots[8];
} light_;

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

	sampler2D mParallax0;
	float mShininess;
} material_;

in V_OUT_ {
	vec3 mNormal;
    vec4 mMVPPos;
    vec3 mMVPos;
    vec3 mMPos;
    vec2 mUV;
	mat3 mTBN;
} v_out_;

out vec4 color_;

//	计算漫反射缩放因子
float CalculateDiffuseScale(vec3 fragNormal, vec3 lightNormal, vec3 cameraNormal)
{
	return max(dot(fragNormal, lightNormal), 0);
}

//	计算镜面反射缩放因子
float CalculateSpecularScale(vec3 fragNormal, vec3 lightNormal, vec3 cameraNormal)
{
	vec3 h = (lightNormal + cameraNormal) * 0.5f;
	return pow(max(dot(fragNormal, h), 0), material_.mShininess);
}

//	计算距离衰减缩放因子
float CalculateDistanceScale(vec3 fragPosition, vec3 lightPosition, float k0, float k1, float k2)
{
	float len =	length(fragPosition - lightPosition);
	return 1.0f / (k0 + k1 * len + k2 * len * len);
}

//	计算外锥衰减缩放因子
float CalculateOutConeScale(float inCone, float outCone, vec3 lightNormal, vec3 lightToFrag)
{
	float cone = dot(lightNormal, lightToFrag);
	return clamp((cone - outCone) / (inCone - outCone), 0, 1);
}

vec2 CalculateParallaxUV(vec3 cameraNormal)
{
	float h = texture(material_.mParallax0, v_out_.mUV).r;
	vec2 offset = cameraNormal.xy * h;
	return v_out_.mUV - offset;
}

vec3 CalculateDirect(LightDirect_ light, vec3 fragNormal, vec3 cameraNormal, vec2 parallaxUV)
{
	float diff = CalculateDiffuseScale(fragNormal, -light.mNormal, cameraNormal);
	float spec = CalculateSpecularScale(fragNormal, -light.mNormal, cameraNormal);

	vec3 ambient = light.mAmbient * texture(material_.mDiffuse0, parallaxUV).rgb;
	vec3 diffuse = light.mDiffuse * texture(material_.mDiffuse0, parallaxUV).rgb * diff;
	vec3 specular = light.mSpecular * texture(material_.mSpecular0, parallaxUV).rgb * spec;
	return ambient + diffuse + specular;
}

vec3 CalculatePoint(LightPoint_ light, vec3 fragNormal, vec3 cameraNormal, vec2 parallaxUV)
{
	vec3 lightNormal = normalize(light.mPosition - v_out_.mMPos);
	float diff = CalculateDiffuseScale(fragNormal, lightNormal, cameraNormal);
	float spec = CalculateSpecularScale(fragNormal, lightNormal, cameraNormal);

	vec3 ambient = light.mAmbient * texture(material_.mDiffuse0, parallaxUV).rgb;
	vec3 diffuse = light.mDiffuse * texture(material_.mDiffuse0, parallaxUV).rgb * diff;
	vec3 specular = light.mSpecular * texture(material_.mSpecular0, parallaxUV).rgb * spec;

	float distance = CalculateDistanceScale(v_out_.mMPos, light.mPosition, light.mK0, light.mK1, light.mK2);

	return (ambient + diffuse + specular) * distance;
}

vec3 CalculateSpot(LightSpot_ light, vec3 fragNormal, vec3 cameraNormal, vec2 parallaxUV)
{
	vec3 lightNormal = normalize(light.mPosition - v_out_.mMPos);

	float diff = CalculateDiffuseScale(fragNormal, lightNormal, cameraNormal);
	float spec = CalculateSpecularScale(fragNormal, lightNormal, cameraNormal);
	
	vec3 ambient = light.mAmbient * texture(material_.mDiffuse0, parallaxUV).rgb;
	vec3 diffuse = light.mDiffuse * texture(material_.mDiffuse0, parallaxUV).rgb * diff;
	vec3 specular = light.mSpecular * texture(material_.mSpecular0, parallaxUV).rgb * spec;

	//	光锥衰减
	float weight = CalculateOutConeScale(light.mInCone, light.mOutCone, light.mNormal, -lightNormal);

	//	距离衰减
	float distance = CalculateDistanceScale(v_out_.mMPos, light.mPosition, light.mK0, light.mK1, light.mK2);

	return (ambient + diffuse + specular) * distance * weight;
}

void main()
{
	vec3 outColor = vec3(0, 0, 0);
	vec3 cameraNormal = normalize(camera_pos_ - v_out_.mMPos);
	vec2 parallaxUV = CalculateParallaxUV(cameraNormal);
	vec3 fragNormal = vec3(texture(material_.mNormal0, parallaxUV));
		 fragNormal = v_out_.mTBN * normalize(fragNormal * 2 - 1.0);

	for (int i = 0; i != light_.mDirectNum; ++i)
	{
		outColor += CalculateDirect(light_.mDirects[i], fragNormal, cameraNormal, parallaxUV);
	}

	for (int i = 0; i != light_.mPointNum; ++i)
	{
		outColor += CalculatePoint(light_.mPoints[i], fragNormal, cameraNormal, parallaxUV);
	}

	for (int i = 0; i != light_.mSpotNum; ++i)
	{
		outColor += CalculateSpot(light_.mSpots[i], fragNormal, cameraNormal, parallaxUV);
	}

	color_ = vec4(outColor, 1.0f);
}