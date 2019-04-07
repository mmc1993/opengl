Pass
CullFace Back
DepthTest
DepthWrite
RenderQueue Geometric
RenderType Forward
DrawType Index

Vertex
#version 410 core

layout(location = 0) in vec3 a_pos_;
layout(location = 1) in vec2 a_uv_;
layout(location = 2) in vec3 a_n_;
layout(location = 3) in vec3 a_t_;
layout(location = 4) in vec3 a_b_;

uniform vec3 camera_pos_;
uniform vec3 camera_eye_;
uniform mat4 matrix_mvp_;
uniform mat4 matrix_mv_;
uniform mat4 matrix_m_;
uniform mat4 matrix_p_;
uniform mat4 matrix_v_;
uniform mat3 matrix_n_;

out V_OUT_ {
    vec3 mNormal;
    vec4 mMVPPos;
    vec3 mMVPos;
    vec3 mMPos;
	mat3 mTBNR;
	mat3 mTBN;
    vec2 mUV;
} v_out_;

void main()
{
	vec4 apos = vec4(a_pos_, 1);
    v_out_.mMPos = vec3(matrix_m_ * apos);
    v_out_.mMVPos = vec3(matrix_mv_ * apos);
	v_out_.mNormal = vec3(matrix_n_ * a_n_);
    v_out_.mMVPPos = vec4(matrix_mvp_ * apos);

	vec3 T = normalize(vec3(matrix_m_ * vec4(a_t_, 0.0f)));
	vec3 B = normalize(vec3(matrix_m_ * vec4(a_b_, 0.0f)));
	vec3 N = normalize(vec3(matrix_m_ * vec4(a_n_, 0.0f)));
	v_out_.mTBN = mat3(T, B, N);
	v_out_.mTBNR = transpose(v_out_.mTBN);

    v_out_.mUV = a_uv_;

    gl_Position = v_out_.mMVPPos;
}
End Vertex

Fragment
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
	mat4 mShadowMat0;
	mat4 mShadowMat1;
	mat4 mShadowMat2;
	mat4 mShadowMat3;
	mat4 mShadowMat4;
	mat4 mShadowMat5;
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

in V_OUT_{
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
	if (vec.x < -vec.w) ret |= 1;
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
	float depth = texture(light_.mDirects[i].mShadowTex, position.xy + vec2(-texstep.x, texstep.y)).r; shadow += position.z < depth ? 1 : 0;
	depth = texture(light_.mDirects[i].mShadowTex, position.xy + vec2(0, texstep.y)).r; shadow += position.z < depth ? 1 : 0;
	depth = texture(light_.mDirects[i].mShadowTex, position.xy + vec2(texstep.x, texstep.y)).r; shadow += position.z < depth ? 1 : 0;
	depth = texture(light_.mDirects[i].mShadowTex, position.xy + vec2(-texstep.x, 0)).r; shadow += position.z < depth ? 1 : 0;
	depth = texture(light_.mDirects[i].mShadowTex, position.xy + vec2(0, 0)).r; shadow += position.z < depth ? 1 : 0;
	depth = texture(light_.mDirects[i].mShadowTex, position.xy + vec2(texstep.x, 0)).r; shadow += position.z < depth ? 1 : 0;
	depth = texture(light_.mDirects[i].mShadowTex, position.xy + vec2(-texstep.x, -texstep.y)).r; shadow += position.z < depth ? 1 : 0;
	depth = texture(light_.mDirects[i].mShadowTex, position.xy + vec2(0, -texstep.y)).r; shadow += position.z < depth ? 1 : 0;
	depth = texture(light_.mDirects[i].mShadowTex, position.xy + vec2(texstep.x, -texstep.y)).r; shadow += position.z < depth ? 1 : 0;
	return shadow / 9.0f;
}

float CalculateSpotShadow(const int i)
{
	vec4 position = light_.mSpots[i].mShadowMat * vec4(v_out_.mMPos, 1);
	if (CheckInView(position) != 0) { return 0; }

	float shadow = 0.0f;
	position.xyz = position.xyz / position.w * 0.5f + 0.5f;
	vec2 texstep = 1.0f / textureSize(light_.mSpots[i].mShadowTex, 0);
	float depth = texture(light_.mSpots[i].mShadowTex, position.xy + vec2(-texstep.x, texstep.y)).r; shadow += position.z < depth ? 1 : 0;
	depth = texture(light_.mSpots[i].mShadowTex, position.xy + vec2(0, texstep.y)).r; shadow += position.z < depth ? 1 : 0;
	depth = texture(light_.mSpots[i].mShadowTex, position.xy + vec2(texstep.x, texstep.y)).r; shadow += position.z < depth ? 1 : 0;
	depth = texture(light_.mSpots[i].mShadowTex, position.xy + vec2(-texstep.x, 0)).r; shadow += position.z < depth ? 1 : 0;
	depth = texture(light_.mSpots[i].mShadowTex, position.xy + vec2(0, 0)).r; shadow += position.z < depth ? 1 : 0;
	depth = texture(light_.mSpots[i].mShadowTex, position.xy + vec2(texstep.x, 0)).r; shadow += position.z < depth ? 1 : 0;
	depth = texture(light_.mSpots[i].mShadowTex, position.xy + vec2(-texstep.x, -texstep.y)).r; shadow += position.z < depth ? 1 : 0;
	depth = texture(light_.mSpots[i].mShadowTex, position.xy + vec2(0, -texstep.y)).r; shadow += position.z < depth ? 1 : 0;
	depth = texture(light_.mSpots[i].mShadowTex, position.xy + vec2(texstep.x, -texstep.y)).r; shadow += position.z < depth ? 1 : 0;
	return shadow / 9.0f;
}

vec4 CalculatePointNDC(const int i, vec4 pos)
{
	vec4 ndc = light_.mPoints[i].mShadowMat0 * pos; if (CheckInView(ndc) == 0) return ndc;
	ndc = light_.mPoints[i].mShadowMat1 * pos; if (CheckInView(ndc) == 0) return ndc;
	ndc = light_.mPoints[i].mShadowMat2 * pos; if (CheckInView(ndc) == 0) return ndc;
	ndc = light_.mPoints[i].mShadowMat3 * pos; if (CheckInView(ndc) == 0) return ndc;
	ndc = light_.mPoints[i].mShadowMat4 * pos; if (CheckInView(ndc) == 0) return ndc;
	ndc = light_.mPoints[i].mShadowMat5 * pos; if (CheckInView(ndc) == 0) return ndc;
	return ndc;
}

float CalculatePointShadow(const int i)
{
	vec3 normal = v_out_.mMPos - light_.mPoints[i].mPosition;
	vec4 sampla = CalculatePointNDC(i, vec4(v_out_.mMPos, 1));
	float depth = texture(light_.mPoints[i].mShadowTex, normalize(normal)).r;
	sampla.z = sampla.z / sampla.w * 0.5f + 0.5f;
	return sampla.z < depth ? 1 : 0;
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
	float len = length(fragPosition - lightPosition);
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
	float shadow = 1;

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
	float shadow = 1;

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
End Fragment

End Pass