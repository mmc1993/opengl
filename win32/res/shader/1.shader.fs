#version 410 core

uniform sampler2D texture0_;
uniform sampler2D texture1_;
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
	vec3 mAmbient;
	vec3 mDiffuse;
	vec3 mSpecular;
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
	color_ = vec4(1.0, 0.5, 0.31, 1);

	vec3 fragNormal = normalize(v_out_.mNormal);
	vec3 lightNormal = normalize(light_.mPosition - v_out_.mMPos);
	vec3 cameraNormal = normalize(camera_pos_ - v_out_.mMPos);

	vec4 ambient = color_ * light_.mAmbient * material_.mAmbient;

	float dDotN = max(dot(lightNormal, fragNormal), 0);
	vec4 diffuse = color_ * light_.mDiffuse * material_.mDiffuse * dDotN;

	vec3 mid = (lightNormal + cameraNormal) * 0.5;
	float power = pow(max(dot(normalize(mid), fragNormal), 0), material_.mShininess);
	vec4 specular = color_ * light_.mSpecular * material_.mSpecular * power;

	color_ = ambient + diffuse + specular;
	// color_ = ambient;
	// color_ = diffuse;
	// color_ = specular;
}