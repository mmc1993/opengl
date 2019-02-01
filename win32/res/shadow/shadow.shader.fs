#version 330 core

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
	LightDirect_ mDirects[2];
	LightPoint_ mPoints[4];
	LightSpot_ mSpots[4];
	sampler2D mDirect0ShadowTex;
	sampler2D mDirect1ShadowTex;
	samplerCube mPoint0ShadowTex;
	samplerCube mPoint1ShadowTex;
	samplerCube mPoint2ShadowTex;
	samplerCube mPoint3ShadowTex;
	sampler2D mSpot0ShadowTex;
	sampler2D mSpot1ShadowTex;
	sampler2D mSpot2ShadowTex;
	sampler2D mSpot3ShadowTex;
	mat4 mDirect0ShadowMat;
	mat4 mDirect1ShadowMat;
	mat4 mSpot0ShadowMat;
	mat4 mSpot1ShadowMat;
	mat4 mSpot2ShadowMat;
	mat4 mSpot3ShadowMat;
} light_;

uniform struct Material_ {
	sampler2D mNormal0;

	sampler2D mDiffuse0;
	sampler2D mDiffuse1;
	sampler2D mDiffuse2;
	sampler2D mDiffuse3;

	sampler2D mSpecular0;
	sampler2D mSpecular1;
	sampler2D mSpecular2;
	sampler2D mSpecular3;
	float mShininess;
} material_;

in V_OUT_ {
	vec3 mNormal;
    vec4 mMVPPos;
    vec3 mMVPos;
    vec3 mMPos;
    vec2 mUV;
} v_out_;

out vec4 color_;

#define CAL_PLANE_SHADOW(outvar, shadowMat, shadowTex)		   {\
		vec2 offset = 1.0f / textureSize(shadowTex, 0);			\
		vec4 coord = shadowMat * vec4(v_out_.mMPos, 1);			\
		coord.xyz = coord.xyz / coord.w * 0.5 + 0.5;			\
		float depth = 0.0f;										\
		depth = texture(shadowTex, coord.xy + vec2(-offset.x,  offset.y)).r; outvar += (depth == 0.0f || depth > coord.z)? 1.0f: 0.0f;	\
		depth = texture(shadowTex, coord.xy + vec2(		0.0f,  offset.y)).r; outvar += (depth == 0.0f || depth > coord.z)? 1.0f: 0.0f;	\
		depth = texture(shadowTex, coord.xy + vec2( offset.x,  offset.y)).r; outvar += (depth == 0.0f || depth > coord.z)? 1.0f: 0.0f;	\
		depth = texture(shadowTex, coord.xy + vec2(-offset.x,	   0.0f)).r; outvar += (depth == 0.0f || depth > coord.z)? 1.0f: 0.0f;	\
		depth = texture(shadowTex, coord.xy + vec2(		0.0f,	   0.0f)).r; outvar += (depth == 0.0f || depth > coord.z)? 1.0f: 0.0f;	\
		depth = texture(shadowTex, coord.xy + vec2( offset.x,	   0.0f)).r; outvar += (depth == 0.0f || depth > coord.z)? 1.0f: 0.0f;	\
		depth = texture(shadowTex, coord.xy + vec2(-offset.x, -offset.y)).r; outvar += (depth == 0.0f || depth > coord.z)? 1.0f: 0.0f;	\
		depth = texture(shadowTex, coord.xy + vec2(		0.0f, -offset.y)).r; outvar += (depth == 0.0f || depth > coord.z)? 1.0f: 0.0f;	\
		depth = texture(shadowTex, coord.xy + vec2( offset.x, -offset.y)).r; outvar += (depth == 0.0f || depth > coord.z)? 1.0f: 0.0f;	\
		outvar = outvar / 9.0f;	}



float CalculateDirectShadow(const int i)
{
	float shadow = 0.0f;
	switch (i)
	{
	case 0: CAL_PLANE_SHADOW(shadow, light_.mDirect0ShadowMat, light_.mDirect0ShadowTex); break;
	case 1: CAL_PLANE_SHADOW(shadow, light_.mDirect1ShadowMat, light_.mDirect1ShadowTex); break;
	}
	return shadow;
}

float CalculatePointShadow(const int i)
{
	float shadow = 0.0f;
	vec3 depthNormal = normalize(v_out_.mMPos - light_.mPoints[i].mPosition);
	switch (i)
	{
	case 0: 
		{
			
			shadow = 1;
		}
		break;
	case 1:
	break;
	case 2:
	break;
	case 3:
	break;
	}
	return shadow;
}

float CalculateSpotShadow(const int i)
{
	float shadow = 0.0f;
	switch (i)
	{
	case 0: CAL_PLANE_SHADOW(shadow, light_.mSpot0ShadowMat, light_.mSpot0ShadowTex); break;
	case 1: CAL_PLANE_SHADOW(shadow, light_.mSpot1ShadowMat, light_.mSpot1ShadowTex); break;
	case 2: CAL_PLANE_SHADOW(shadow, light_.mSpot2ShadowMat, light_.mSpot2ShadowTex); break;
	case 3: CAL_PLANE_SHADOW(shadow, light_.mSpot3ShadowMat, light_.mSpot3ShadowTex); break;
	}
	return shadow;
}

vec3 CalculateDirect(const int i, vec3 fragNormal, vec3 viewNormal)
{
	float shadow = CalculateDirectShadow(i);

	vec3 center = (-light_.mDirects[i].mNormal + viewNormal) * 0.5;
	float diff	= max(0, dot(fragNormal, -light_.mDirects[i].mNormal));
	float spec	= pow(max(0, dot(fragNormal, center)), material_.mShininess);

	vec3 ambient = light_.mDirects[i].mAmbient * texture(material_.mDiffuse0, v_out_.mUV).rgb;
	vec3 diffuse = light_.mDirects[i].mDiffuse * texture(material_.mDiffuse0, v_out_.mUV).rgb * diff;
	vec3 specular = light_.mDirects[i].mSpecular * texture(material_.mSpecular0, v_out_.mUV).rgb * spec;
	return ambient + diffuse * shadow + specular * shadow;
}

vec3 CalculatePoint(const int i, vec3 fragNormal, vec3 viewNormal)
{
	

	vec3 lightNormal = normalize(light_.mPoints[i].mPosition - v_out_.mMPos);
	vec3 center = (lightNormal + viewNormal) * 0.5;
	float diff = max(0, dot(fragNormal, lightNormal));
	float spec = pow(max(0, dot(fragNormal, center)), material_.mShininess);

	vec3 ambient = light_.mPoints[i].mAmbient * texture(material_.mDiffuse0, v_out_.mUV).rgb;
	vec3 diffuse = light_.mPoints[i].mDiffuse * texture(material_.mDiffuse0, v_out_.mUV).rgb * diff;
	vec3 specular = light_.mPoints[i].mSpecular * texture(material_.mSpecular0, v_out_.mUV).rgb * spec;

	float distance = length(light_.mPoints[i].mPosition - v_out_.mMPos);
	float weight = 1 / (light_.mPoints[i].mK0 + light_.mPoints[i].mK1 * distance + light_.mPoints[i].mK2 * distance * distance);

	return (ambient + diffuse + specular) * weight;
}

vec3 CalculateSpot(const int i, vec3 fragNormal, vec3 viewNormal)
{
	vec3 lightNormal	= normalize(light_.mSpots[i].mPosition - v_out_.mMPos);
	float fragCone		= max(0, dot(lightNormal, -light_.mSpots[i].mNormal));
	float diffCone		= light_.mSpots[i].mInCone - light_.mSpots[i].mOutCone;
	float cutWeight		= clamp((fragCone - light_.mSpots[i].mOutCone) / diffCone, 0, 1);
	if (cutWeight == 0) { return vec3(0, 0, 0); }

	float shadow = CalculateSpotShadow(i);

	vec3 center = (lightNormal + viewNormal) * 0.5;
	float diff = max(0, dot(fragNormal, lightNormal));
	float spec = pow(max(0, dot(fragNormal, center)), material_.mShininess);

	vec3 ambient = light_.mSpots[i].mAmbient * texture(material_.mDiffuse0, v_out_.mUV).rgb;
	vec3 diffuse = light_.mSpots[i].mDiffuse * texture(material_.mDiffuse0, v_out_.mUV).rgb * diff;
	vec3 specular = light_.mSpots[i].mSpecular * texture(material_.mSpecular0, v_out_.mUV).rgb * spec;

	float distance = length(light_.mSpots[i].mPosition - v_out_.mMPos);
	float weight = 1 / (light_.mSpots[i].mK0 + light_.mSpots[i].mK1 * distance + light_.mSpots[i].mK2 * distance * distance);

	return (ambient + diffuse * shadow + specular * shadow) * weight * cutWeight;
}

void main()
{
	vec3 outColor = vec3(0, 0, 0);
	vec3 viewNormal = normalize(camera_pos_ - v_out_.mMPos);
	for (int i = 0; i != light_.mDirectNum; ++i)
	{
		outColor += CalculateDirect(i, v_out_.mNormal, viewNormal);
	}

	for (int i = 0; i != light_.mPointNum; ++i)
	{
		outColor += CalculatePoint(i, v_out_.mNormal, viewNormal);
	}

	for (int i = 0; i != light_.mSpotNum; ++i)
	{
		outColor += CalculateSpot(i, v_out_.mNormal, viewNormal);
	}
	
	color_ = vec4(outColor, 1);
}