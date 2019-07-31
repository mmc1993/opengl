#pragma once

#include "../include.h"
#include "../raw/gl_mesh.h"
#include "../raw/gl_program.h"
#include "../raw/gl_material.h"
#include "../raw/gl_texture2d.h"

enum RenderQueueEnum {
	kBACKGROUND,        //  底层绘制
	kGEOMETRIC,         //  常规绘制
	kOPACITY,           //  透明绘制
	kTOP,               //  顶层绘制
};

enum RenderTypeEnum {
    kLIGHT,             //  渲染光源
    kDEPTH,             //  深度渲染
    kSHADOW,            //  阴影贴图
	kFORWARD,           //  正向渲染
	kDEFERRED,          //  延迟渲染
};

enum DrawTypeEnum {
	kINSTANCE,		    //	实例draw
	kVERTEX,		    //	顶点draw
	kINDEX,			    //	索引draw
};

enum FragTypeEnum {
    kTRIANGLE   = GL_TRIANGLES,
    kPOINT      = GL_POINTS,
    kLINE       = GL_LINES,
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
//  视图投影矩阵
static const char * const UNIFORM_MATRIX_VP         = "matrix_vp_";
//  模型视图投影矩阵
static const char * const UNIFORM_MATRIX_MVP        = "matrix_mvp_";
//  视图矩阵(逆)
static const char * const UNIFORM_MATRIX_V_INV      = "matrix_v_inv_";
//  投影矩阵(逆)
static const char * const UNIFORM_MATRIX_P_INV      = "matrix_p_inv_";
//  模型视图矩阵(逆)
static const char * const UNIFORM_MATRIX_MV_INV     = "matrix_mv_inv_";
//  视图投影矩阵(逆)
static const char * const UNIFORM_MATRIX_VP_INV     = "matrix_vp_inv_";
//  游戏时间
static const char * const UNIFORM_GAME_TIME         = "game_time_";

//  分辨率尺寸
static const char * const UNIFORM_VIEW_SIZE         = "view_size_";
//  相机近裁剪距离
static const char * const UNIFORM_CAMERA_N          = "camera_n_";
//  相机远裁剪距离
static const char * const UNIFORM_CAMERA_F          = "camera_f_";
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

//  SSAO
static const char * const UNIFORM_SCREEN_POSTION        = "screen_position_";
static const char * const UNIFORM_SCREEN_DEPTH          = "screen_depth_";
static const char * const UNIFORM_SCREEN_SAO            = "screen_sao_";

//  内置资源-延迟渲染-光体积
static const char * const BUILTIN_MESH_DEFERRED_LIGHT_VOLUME_DIRECT = "res/built-in/mesh/deferred_light_volume_direct.obj";
static const char * const BUILTIN_MESH_DEFERRED_LIGHT_VOLUME_POINT  = "res/built-in/mesh/deferred_light_volume_point.obj";
static const char * const BUILTIN_MESH_DEFERRED_LIGHT_VOLUME_SPOT   = "res/built-in/mesh/deferred_light_volume_spot.obj";
static const char * const BUILTIN_MESH_SCREEN_QUAD                  = "res/built-in/mesh/screen_quad.obj";
//  SSAO 着色器
static const char * const BUILTIN_PROGRAM_SSAO                      = "res/built-in/program/ssao.program";

//  矩阵栈
class MatrixStack {
public:
    enum ModeType { kPROJ, kVIEW, kMODEL, };

public:
    MatrixStack()
    { }

    ~MatrixStack()
    { }

    void Pop(ModeType mode)
    {
        GetStack(mode).pop();
    }

    void Push(ModeType mode)
    {
        GetStack(mode).push(GetStack(mode).top());
    }

    void Identity(ModeType mode)
    {
        GetStack(mode).push(glm::mat4(1));
    }

    void Mul(ModeType mode, const glm::mat4 & mat)
    {
        GetStack(mode).top() *= mat;
    }

    const glm::mat4 & Top(ModeType mode) const
    {
        return GetStack(mode).top();
    }

    const glm::mat4 & GetM() const
    {
        return Top(ModeType::kMODEL);
    }

    const glm::mat4 & GetV() const
    {
        return Top(ModeType::kVIEW);
    }

    const glm::mat4 & GetP() const
    {
        return Top(ModeType::kPROJ);
    }

private:
    std::stack<glm::mat4> & GetStack(ModeType mode) const
    {
        return _matrixs.at((size_t)mode);
    }

private:
    mutable std::array<std::stack<glm::mat4>, 3> _matrixs;
};

enum class CommandEnum {
    kMATERIAL,
    kCAMERA,
    kLIGHT,
};
//  用于渲染的命令结构
struct RenderCommand { };

//  材质渲染命令
struct MaterialCommand : public RenderCommand {
    MaterialCommand() 
        : mProgramParam(nullptr)
        , mMaterial(nullptr)
    { }
    const GLProgramParam  * mProgramParam;
    const GLMaterial      * mMaterial;
    glm::mat4               mTransform;
    uint                    mCameraMask;
    uint                    mSubPass;
};

//  相机渲染命令
struct CameraCommand : public RenderCommand {
    CameraCommand() { }
    glm::vec4   mViewport;
    glm::mat4   mProj;
    glm::mat4   mView;
    glm::vec3   mPos;
    glm::vec3   mEye;
    float       mN, mF;
    uint        mOrder;
    uint        mMask;

    bool operator<(const CameraCommand & other) const
    {
        return mOrder < other.mOrder;
    }
};

//  光源渲染命令
struct LightCommand : public RenderCommand {
    LightCommand() 
        : mMesh(nullptr)
        , mProgram(nullptr)
    { }
    glm::vec3   mPosition;
    glm::mat4   mTransform;
    glm::mat4   mView;
    glm::mat4   mProj;
    uint        mType;
    uint        mUBO;
    GLMesh    * mMesh;
    GLProgram * mProgram;
};
