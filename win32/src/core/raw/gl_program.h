#pragma once

#include "gl_res.h"

class GLProgram : public GLRes {
public:
    struct PassAttr {
        //  面剔除
        int     vCullFace;
        //  混合
        int     vBlendSrc;
        int     vBlendDst;
        //  深度测试
        bool    bDepthTest;         //  开启深度测试
        bool    bDepthWrite;        //  开启深度写入
        //  模板测试
        int     vStencilOpFail;		//	模板测试失败
        int     vStencilOpZFail;	//	深度测试失败
        int     vStencilOpZPass;	//	深度测试通过
        int		vStencilFunc;		//	模板测试函数
        int     vStencilMask;       //  模板测试值
        int     vStencilRef;        //  模板测试值
        //  渲染
        uint    vDrawType;			//	draw类型
        uint    vRenderType;        //  渲染类型
        uint    vRenderQueue;       //  渲染通道
    };
public:
    GLProgram()
    { }

    ~GLProgram()
    { 
        glDeleteProgram(_id);
    }

    void Init(const char * vString, const char * gString, const char * fString)
    {
        _id = glCreateProgram();

        if (vString != nullptr)
        {
            uint vs = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vs, 1, &vString, nullptr);
            glCompileShader(vs);
            AssertPass(vs, "VShader Error");
            glAttachShader(_id, vs);
            glDeleteShader(vs);
        }

        if (gString != nullptr)
        {
            uint gs = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(gs, 1, &gString, nullptr);
            glCompileShader(gs);
            AssertPass(gs, "GShader Error");
            glAttachShader(_id, gs);
            glDeleteShader(gs);
        }

        if (gString != nullptr)
        {
            uint fs = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fs, 1, &gString, nullptr);
            glCompileShader(fs);
            AssertPass(fs, "FShader Error");
            glAttachShader(_id, fs);
            glDeleteShader(fs);
        }

        iint ret;
        glLinkProgram(_id);
        glGetProgramiv(_id, GL_LINK_STATUS, &ret);
        ASSERT_LOG(ret != 0, "GLProgram Error");
    }

    void AddPassAttr(const PassAttr & passAttr)
    {
        _passAttrs.push_back(passAttr);
    }

    const PassAttr & GetPassAttr(uint i) const
    {
        return _passAttrs.at(i);
    }

    void UsePass(uint i) const
    {
        //  TODO
    }

    void Use() const
    {
        glUseProgram(_id);
    }

    //void Bind();

private:
    void AssertPass(uint shaderID, const std::string & errorTxt)
    {
        iint ret;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &ret);
        if (ret == 0)
        {
            char err[256] = { 0 };
            glGetShaderInfoLog(shaderID, sizeof(err), nullptr, err);
            ASSERT_LOG(false, "Shader Error. {0}, Code: {1}, Text: {2}", errorTxt, ret, err);
        }
    }

private:
    std::vector<PassAttr> _passAttrs;
    uint _id;
};