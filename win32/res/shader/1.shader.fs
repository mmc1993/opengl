#version 410 core

uniform vec3 camera_pos_;
uniform vec3 camera_eye_;
uniform mat4 mvp_;
uniform mat4 mv_;
uniform mat4 m_;

uniform struct Light_ {
	vec3 mAmbient;
	vec3 mDiffuse;
	vec3 mSpecular;
	vec3 mPosition;
} light_;

uniform struct Material_ {
	sampler2D mDiffuse;
	sampler2D mSpecular;
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

void main()
{
	vec3 fragNormal = normalize(v_out_.mNormal);
	vec3 lightNormal = normalize(light_.mPosition - v_out_.mMPos);
	vec3 cameraNormal = normalize(camera_pos_ - v_out_.mMPos);

	vec3 ambient = light_.mAmbient * texture(material_.mDiffuse, v_out_.mUV).rgb;

	float dDotN = max(dot(lightNormal, fragNormal), 0);
	vec3 diffuse = light_.mDiffuse * texture(material_.mDiffuse, v_out_.mUV).rgb * dDotN;

	float lDotF = max(dot(normalize((lightNormal + cameraNormal) * 0.5), fragNormal), 0);
	vec3 specular = light_.mSpecular * texture(material_.mSpecular, v_out_.mUV).rgb * pow(lDotF, material_.mShininess);

	color_ = vec4(ambient + diffuse + specular, 1);
}