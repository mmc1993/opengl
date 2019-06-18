CullFace    Front
DepthTest
DepthWrite
RenderType  Shadow
DrawType    Index

VShader Beg

    layout(location = 0) in vec3 a_v_;

    uniform mat4 matrix_mvp_;
    uniform mat4 matrix_m_;

    out V_OUT_ {
        vec3 mMPos;
    } v_out_;

    void main()
    {
        vec4 apos       = vec4(a_v_, 1);
        v_out_.mMPos    = vec3(matrix_m_ * apos);
        gl_Position     = vec4(matrix_mvp_ * apos);
    }
    VShader End

    FShader Beg

    layout (std140) uniform LightPoint_ {
        float mFar, mNear;
        float mK0, mK1, mK2;
        vec3 mAmbient;
        vec3 mDiffuse;
        vec3 mSpecular;
        vec3 mPosition;
    } light_point_;

    in V_OUT_{
        vec3 mMPos;
    } v_out_;

    uniform int light_type_;

    #define LIGHT_TYPE_DIRECT_ 0
    #define LIGHT_TYPE_POINT_ 1
    #define LIGHT_TYPE_SPOT_ 2

    void main()
    {
        switch (light_type_)
        {
        case LIGHT_TYPE_DIRECT_:
            {
                gl_FragDepth = gl_FragCoord.z;
            }
            break;
        case LIGHT_TYPE_POINT_:
            {
                vec3 normal  = v_out_.mMPos   - light_point_.mPosition;
                gl_FragDepth = length(normal) / light_point_.mFar;
            }
            break;
        case LIGHT_TYPE_SPOT_:
            {
                gl_FragDepth = gl_FragCoord.z;
            }
            break;
        }
    }
FShader End