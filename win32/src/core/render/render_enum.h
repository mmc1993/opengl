#pragma once

enum RenderQueueEnum {
	kBACKGROUND,    //  底层绘制
	kGEOMETRIC,     //  常规绘制
	kOPACITY,       //  透明绘制
	kTOP,           //  顶层绘制
};

enum RenderTypeEnum {
    kLIGHT,         //  渲染光源
	kSHADOW,        //  阴影贴图
	kFORWARD,       //  正向渲染
	kDEFERRED,      //  延迟渲染
};

enum DrawTypeEnum {
	kINSTANCE,		//	实例draw
	kVERTEX,		//	顶点draw
	kINDEX,			//	索引draw
};

enum UniformBlockEnum {
    kLIGHT_DIRECT,      //  方向光绑定点
    kLIGHT_POINT,       //  点光源绑定点
    kLIGHT_SPOT,        //  聚光灯绑定点
};

//  UBO 方向光数据
static const char * const UBO_NAME_LIGHT_DIRECT     = "LightDirect_";
//  UBO 点光源数据
static const char * const UBO_NAME_LIGHT_POINT      = "LightPoint_";
//  UBO 聚光灯数据
static const char * const UBO_NAME_LIGHT_SPOT       = "LightSpot_";

//  光源类型
static const char * const UNIFORM_LIGHT_TYPE        = "light_type_";
//  光源位置
static const char * const UNIFORM_LIGHT_POS         = "light_pos_";
//  法线矩阵
static const char * const UNIFORM_MATRIX_N          = "matrix_n_";
//  模型矩阵
static const char * const UNIFORM_MATRIX_M          = "matrix_m_";
//  视图矩阵
static const char * const UNIFORM_MATRIX_V          = "matrix_v_";
//  投影矩阵
static const char * const UNIFORM_MATRIX_P          = "matrix_p_";
//  模型视图矩阵
static const char * const UNIFORM_MATRIX_MV         = "matrix_mv_";
//  模型视图投影矩阵
static const char * const UNIFORM_MATRIX_MVP        = "matrix_mvp_";
//  游戏时间
static const char * const UNIFORM_GAME_TIME         = "game_time_";

//  相机位置
static const char * const UNIFORM_CAMERA_POS        = "camera_pos_";
//  相机朝向
static const char * const UNIFORM_CAMERA_EYE        = "camera_eye_";

//  材质*
static const char * const UNIFORM_MATERIAL              = "material_.{0}";
//  方向光阴影贴图
static const char * const UNIFORM_SHADOW_MAP_DIRECT_    = "shadow_map_direct_{0}_";
//  点光源阴影贴图
static const char * const UNIFORM_SHADOW_MAP_POINT_     = "shadow_map_point_{0}_";
//  聚光灯阴影贴图
static const char * const UNIFORM_SHADOW_MAP_SPOT_      = "shadow_map_spot_{0}_";
//  方向光数量
static const char * const UNIFORM_LIGHT_COUNT_DIRECT_   = "light_count_direct_";
//  点光源数量
static const char * const UNIFORM_LIGHT_COUNT_POINT_    = "light_count_point_";
//  聚光灯数量
static const char * const UNIFORM_LIGHT_COUNT_SPOT_     = "light_count_spot_";

//  G-Buffer Position
static const char * const UNIFORM_GBUFFER_POSIITON      = "gbuffer_position_";
static const char * const UNIFORM_GBUFFER_SPECULAR      = "gbuffer_specular_";
static const char * const UNIFORM_GBUFFER_DIFFUSE       = "gbuffer_diffuse_";
static const char * const UNIFORM_GBUFFER_NORMAL        = "gbuffer_normal_";

//  内置资源-延迟渲染-光体积
static const char * const BUILTIN_MESH_DEFERRED_LIGHT_VOLUME_DIRECT = "res/built-in/mesh/deferred_light_volume_direct.obj";
static const char * const BUILTIN_MESH_DEFERRED_LIGHT_VOLUME_POINT  = "res/built-in/mesh/deferred_light_volume_point.obj";
static const char * const BUILTIN_MESH_DEFERRED_LIGHT_VOLUME_SPOT   = "res/built-in/mesh/deferred_light_volume_spot.obj";
