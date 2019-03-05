#version 410 core

uniform vec3 camera_pos_;
uniform vec3 camera_eye_;

struct LightDirect_ {
	vec3 mNormal;
	
	vec3 mAmbient;
	vec3 mDiffuse;
	vec3 mSpecular;
	mat4 mShadowMat;
	sampler2D mShadowTex;
};

struct LightPoint_ {
	vec3 mPosition;
	float mK0, mK1, mK2;
	
	vec3 mAmbient;
	vec3 mDiffuse;
	vec3 mSpecular;
	float mNear, mFar;
	samplerCube mShadowTex;
};

struct LightSpot_ {
	vec3 mNormal;
	vec3 mPosition;
	float mK0, mK1, mK2;
	float mOutCone, mInCone;

	vec3 mAmbient;
	vec3 mDiffuse;
	vec3 mSpecular;
	mat4 mShadowMat;
	sampler2D mShadowTex;
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
	float mShininess;
	sampler2D mNormal;
	sampler2D mSpecular;
	sampler2D mDiffuse0;
	sampler2D mDiffuse1;
	sampler2D mDiffuse2;
	sampler2D mDiffuse3;
} material_;

in V_OUT_ {
	vec3 mNormal;
    vec4 mMVPPos;
    vec3 mMVPos;
    vec3 mMPos;
	mat3 mTBNR;
	mat3 mTBN;
    vec2 mUV;
} v_out_;

out vec4 color_;

int CheckInView(vec4 vec)
{
	int ret = 0;
	if		(vec.x < -vec.w) ret |= 1;
	else if (vec.x >  vec.w) ret |= 2;
	else if (vec.y < -vec.w) ret |= 4;
	else if (vec.y >  vec.w) ret |= 8;
	else if (vec.z < -vec.w) ret |= 16;
	else if (vec.z >  vec.w) ret |= 32;
	return ret;
}

float CalculateDirectShadow(const int i)
{
	vec4 position = light_.mDirects[i].mShadowMat * vec4(v_out_.mMPos, 1);
	if (CheckInView(position) != 0) { return 0; }

	float shadow = 0.0f;
	position.xyz = position.xyz / position.w * 0.5f + 0.5f;
	vec2 texstep = 1.0f / textureSize(light_.mDirects[i].mShadowTex, 0);
	float depth = texture(light_.mDirects[i].mShadowTex, position.xy + vec2(-texstep.x,  texstep.y	)).r; shadow += position.z < depth? 1: 0;
		  depth = texture(light_.mDirects[i].mShadowTex, position.xy + vec2( 0,			 texstep.y	)).r; shadow += position.z < depth? 1: 0;
		  depth = texture(light_.mDirects[i].mShadowTex, position.xy + vec2( texstep.x,  texstep.y	)).r; shadow += position.z < depth? 1: 0;
		  depth = texture(light_.mDirects[i].mShadowTex, position.xy + vec2(-texstep.x,	 0			)).r; shadow += position.z < depth? 1: 0;
		  depth = texture(light_.mDirects[i].mShadowTex, position.xy + vec2( 0,			 0			)).r; shadow += position.z < depth? 1: 0;
		  depth = texture(light_.mDirects[i].mShadowTex, position.xy + vec2( texstep.x,	 0			)).r; shadow += position.z < depth? 1: 0;
		  depth = texture(light_.mDirects[i].mShadowTex, position.xy + vec2(-texstep.x,	-texstep.y	)).r; shadow += position.z < depth? 1: 0;
		  depth = texture(light_.mDirects[i].mShadowTex, position.xy + vec2( 0,			-texstep.y	)).r; shadow += position.z < depth? 1: 0;
		  depth = texture(light_.mDirects[i].mShadowTex, position.xy + vec2( texstep.x,	-texstep.y	)).r; shadow += position.z < depth? 1: 0;
	return shadow / 9.0f;
}

float CalculateSpotShadow(const int i)
{
	vec4 position = light_.mSpots[i].mShadowMat * vec4(v_out_.mMPos, 1);
	if (CheckInView(position) != 0) { return 0; }

	float shadow = 0.0f;
	position.xyz = position.xyz / position.w * 0.5f + 0.5f;
	vec2 texstep = 1.0f / textureSize(light_.mSpots[i].mShadowTex, 0);
	float depth = texture(light_.mSpots[i].mShadowTex, position.xy + vec2(-texstep.x,  texstep.y	)).r; shadow += position.z < depth? 1: 0;
		  depth = texture(light_.mSpots[i].mShadowTex, position.xy + vec2( 0,			 texstep.y	)).r; shadow += position.z < depth? 1: 0;
		  depth = texture(light_.mSpots[i].mShadowTex, position.xy + vec2( texstep.x,  texstep.y	)).r; shadow += position.z < depth? 1: 0;
		  depth = texture(light_.mSpots[i].mShadowTex, position.xy + vec2(-texstep.x,	 0			)).r; shadow += position.z < depth? 1: 0;
		  depth = texture(light_.mSpots[i].mShadowTex, position.xy + vec2( 0,			 0			)).r; shadow += position.z < depth? 1: 0;
		  depth = texture(light_.mSpots[i].mShadowTex, position.xy + vec2( texstep.x,	 0			)).r; shadow += position.z < depth? 1: 0;
		  depth = texture(light_.mSpots[i].mShadowTex, position.xy + vec2(-texstep.x,	-texstep.y	)).r; shadow += position.z < depth? 1: 0;
		  depth = texture(light_.mSpots[i].mShadowTex, position.xy + vec2( 0,			-texstep.y	)).r; shadow += position.z < depth? 1: 0;
		  depth = texture(light_.mSpots[i].mShadowTex, position.xy + vec2( texstep.x,	-texstep.y	)).r; shadow += position.z < depth? 1: 0;
	return shadow / 9.0f;
}

float CalculatePointShadow(const int i)
{
	vec3 normal = v_out_.mMPos - light_.mPoints[i].mPosition;
	float depth = 1 / light_.mPoints[i].mNear / (normal.z - 1) /
				  1 / light_.mPoints[i].mNear / (light_.mPoints[i].mFar - 1);
	float value = texture(light_.mPoints[i].mShadowTex, normalize(normal)).r;
	return depth < value? 1: 0;
}

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

vec3 CalculateDirect(const int i, vec3 fragNormal, vec3 cameraNormal, vec2 parallaxUV)
{
	float shadow = CalculateDirectShadow(i);

	float diff = CalculateDiffuseScale(fragNormal, -light_.mDirects[i].mNormal, cameraNormal);
	float spec = CalculateSpecularScale(fragNormal, -light_.mDirects[i].mNormal, cameraNormal);

	vec3 ambient = light_.mDirects[i].mAmbient * texture(material_.mDiffuse0, parallaxUV).rgb;
	vec3 diffuse = light_.mDirects[i].mDiffuse * texture(material_.mDiffuse0, parallaxUV).rgb * diff;
	vec3 specular = light_.mDirects[i].mSpecular * texture(material_.mSpecular, parallaxUV).rgb * spec;
	return ambient + (diffuse + specular) * shadow;
}

vec3 CalculatePoint(const int i, vec3 fragNormal, vec3 cameraNormal, vec2 parallaxUV)
{
	float shadow = CalculatePointShadow(i);

	vec3 lightNormal = normalize(light_.mPoints[i].mPosition - v_out_.mMPos);
	float diff = CalculateDiffuseScale(fragNormal, lightNormal, cameraNormal);
	float spec = CalculateSpecularScale(fragNormal, lightNormal, cameraNormal);

	vec3 ambient = light_.mPoints[i].mAmbient * texture(material_.mDiffuse0, parallaxUV).rgb;
	vec3 diffuse = light_.mPoints[i].mDiffuse * texture(material_.mDiffuse0, parallaxUV).rgb * diff;
	vec3 specular = light_.mPoints[i].mSpecular * texture(material_.mSpecular, parallaxUV).rgb * spec;

	float distance = CalculateDistanceScale(v_out_.mMPos, light_.mPoints[i].mPosition, light_.mPoints[i].mK0, light_.mPoints[i].mK1, light_.mPoints[i].mK2);

	return (ambient + (diffuse + specular) * shadow) * distance;
}

vec3 CalculateSpot(const int i, vec3 fragNormal, vec3 cameraNormal, vec2 parallaxUV)
{
	float shadow = CalculateSpotShadow(i);

	vec3 lightNormal = normalize(light_.mSpots[i].mPosition - v_out_.mMPos);

	float diff = CalculateDiffuseScale(fragNormal, lightNormal, cameraNormal);
	float spec = CalculateSpecularScale(fragNormal, lightNormal, cameraNormal);
	
	vec3 ambient = light_.mSpots[i].mAmbient * texture(material_.mDiffuse0, parallaxUV).rgb;
	vec3 diffuse = light_.mSpots[i].mDiffuse * texture(material_.mDiffuse0, parallaxUV).rgb * diff;
	vec3 specular = light_.mSpots[i].mSpecular * texture(material_.mSpecular, parallaxUV).rgb * spec;

	//	光锥衰减
	float weight = CalculateOutConeScale(light_.mSpots[i].mInCone, light_.mSpots[i].mOutCone, light_.mSpots[i].mNormal, -lightNormal);

	//	距离衰减
	float distance = CalculateDistanceScale(v_out_.mMPos, light_.mSpots[i].mPosition, light_.mSpots[i].mK0, light_.mSpots[i].mK1, light_.mSpots[i].mK2);

	return (ambient + (diffuse + specular) * shadow) * distance * weight;
}

void main()
{
	vec3 cameraNormal = normalize(camera_pos_ - v_out_.mMPos);

	vec3 fragNormal = vec3(texture(material_.mNormal, v_out_.mUV));
		 fragNormal = v_out_.mTBN * normalize(fragNormal * 2 - 1.0);
	
	vec3 outColor = vec3(0, 0, 0);
	for (int i = 0; i != light_.mDirectNum; ++i)
	{
		outColor += CalculateDirect(i, fragNormal, cameraNormal, v_out_.mUV);
	}

	for (int i = 0; i != light_.mPointNum; ++i)
	{
		outColor += CalculatePoint(i, fragNormal, cameraNormal, v_out_.mUV);
	}

	for (int i = 0; i != light_.mSpotNum; ++i)
	{
		outColor += CalculateSpot(i, fragNormal, cameraNormal, v_out_.mUV);
	}

	color_ = vec4(outColor, 1.0f);
}