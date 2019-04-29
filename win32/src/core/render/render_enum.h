#pragma once

enum RenderQueueEnum {
	kBACKGROUND,    //  �ײ����
	kGEOMETRIC,     //  �������
	kOPACITY,       //  ͸������
	kTOP,           //  �������
};

enum RenderTypeEnum {
	kSHADOW,        //  ������Ӱ��ͼ
	kFORWARD,       //  ������Ⱦ
	kDEFERRED,      //  �ӳ���Ⱦ
};

enum DrawTypeEnum {
	kINSTANCE,		//	ʵ��draw
	kVERTEX,		//	����draw
	kINDEX,			//	����draw
};

enum UniformBlockEnum {
    kLIGHT_DIRECT,      //  �����󶨵�
    kLIGHT_POINT,       //  ���Դ�󶨵�
    kLIGHT_SPOT,        //  �۹�ư󶨵�
};

//  UBO ���������
static const char * const UBO_NAME_LIGHT_DIRECT = "LightDirect_";
//  UBO ���Դ����
static const char * const UBO_NAME_LIGHT_POINT = "LightPoint_";
//  UBO �۹������
static const char * const UBO_NAME_LIGHT_SPOT = "LightSpot_";

//  ��Դλ��
static const char * const UNIFORM_LIGHT_POS = "light_pos_";
//  ��Դ����
static const char * const UNIFORM_LIGHT_TYPE = "light_type_";
//  ���߾���
static const char * const UNIFORM_MATRIX_N = "matrix_n_";
//  ģ�;���
static const char * const UNIFORM_MATRIX_M = "matrix_m_";
//  ��ͼ����
static const char * const UNIFORM_MATRIX_V = "matrix_v_";
//  ͶӰ����
static const char * const UNIFORM_MATRIX_P = "matrix_p_";
//  ģ����ͼ����
static const char * const UNIFORM_MATRIX_MV = "matrix_mv_";
//  ģ����ͼͶӰ����
static const char * const UNIFORM_MATRIX_MVP = "matrix_mvp_";
//  ��Ϸʱ��
static const char * const UNIFORM_GAME_TIME = "game_time_";

//  ���λ��
static const char * const UNIFORM_CAMERA_POS = "camera_pos_";
//  �������
static const char * const UNIFORM_CAMERA_EYE = "camera_eye_";

//  ����.������
static const char * const UNIFORM_MATERIAL_DIFFUSE      = "material_.mDiffuse{0}";
//  ����.���淴��
static const char * const UNIFORM_MATERIAL_SPECULAR     = "material_.mSpecular";
//  ����.������ͼ
static const char * const UNIFORM_MATERIAL_REFLECT      = "material_.mReflect";
//  ����.������ͼ
static const char * const UNIFORM_MATERIAL_NORMAL       = "material_.mNormal";
//  ����.�߶���ͼ
static const char * const UNIFORM_MATERIAL_HEIGHT       = "material_.mHeight";
//  ����.�߹�ǿ��
static const char * const UNIFORM_MATERIAL_SHININESS    = "material_.mShininess";
//  2D��Ӱ��ͼ(�����, �۹��)
static const char * const UNIFORM_SHADOW_MAP_2D_        = "shadow_map_2d_";
//  3D��Ӱ��ͼ(���Դ)
static const char * const UNIFORM_SHADOW_MAP_3D_        = "shadow_map_3d_";
//  ���������
static const char * const UNIFORM_LIGHT_COUNT_DIRECT_   = "light_count_direct_";
//  ���Դ����
static const char * const UNIFORM_LIGHT_COUNT_POINT_    = "light_count_point_";
//  �۹������
static const char * const UNIFORM_LIGHT_COUNT_SPOT_     = "light_count_spot_";