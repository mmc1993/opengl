//	frag
#version 410 core

uniform mat4 nmat_;
uniform mat4 mvp_;
uniform mat4 mv_;
uniform vec3 camera_pos_;
uniform vec3 camera_eye_;
uniform sampler2D texture_;
uniform sampler2D normal_;

struct LightPoint_ {
	vec4 mPos;		//	16
	vec4 mColor;	//	32
	float mMin;		//	48
	float mMax;		//	52
};

struct LightSpot_ {
	vec4 mPos;
	vec4 mDir;
	vec4 mColor;
	float mMinCone;
	float mMaxCone;
	float mMin;
	float mMax;
};

layout (std140) uniform Light_ {
	float mAmbient;		//	0
	int mPointNum;		//	4
	int mSpotNum;		//	8
	LightPoint_ mPoints[32];
	LightSpot_ mSpots[32];
} light_;

in vec2 vuv_;
in vec4 vpos_mv_;
in vec4 vpos_mvp_;

out vec4 color_;

