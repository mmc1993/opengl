#pragma once

#include "gl_res.h"

#define MMC_TEXTURE_2D  GL_TEXTURE_2D
#define MMC_TEXTURE_3D  GL_TEXTURE_CUBE_MAP

class GLProgram : public GLRes {
public:
    struct PassAttr {
        //  面剔除
        int     vCullFace;
        //  混合
        int     vBlendSrc;
        int     vBlendDst;
        //  深度测试
        short   bDepthTest;         //  开启深度测试
        short   bDepthWrite;        //  开启深度写入
        //  模板测试
        int     vStencilOpFail;		//	模板测试失败
        int     vStencilOpZFail;	//	深度测试失败
        int     vStencilOpZPass;	//	深度测试通过
        int		vStencilFunc;		//	模板测试函数
        int     vStencilMask;       //  模板测试值
        int     vStencilRef;        //  模板测试值
        //  渲染
        short   vDrawType;			//	绘制类型
        short   vRenderType;        //  渲染类型
        short   vRenderQueue;       //  渲染通道
        char    mPassName[14];      //  Pass名字

        PassAttr() { memset(this, 0, sizeof(PassAttr)); }
    };
public:
    GLProgram()
    { }

    ~GLProgram()
    { 
        glDeleteProgram(_id);
    }

    void Init(
        const char * vString, uint vLength,
        const char * gString, uint gLength,
        const char * fString, uint fLength)
    {
        _id = glCreateProgram();

        if (vLength != 0)
        {
            uint vs = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vs, 1, &vString, (iint*)&vLength);
            glCompileShader(vs);
            AssertPass(vs, "VShader Error");
            glAttachShader(_id, vs);
            glDeleteShader(vs);
        }

        if (gLength != 0)
        {
            uint gs = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(gs, 1, &gString, (iint*)&gLength);
            glCompileShader(gs);
            AssertPass(gs, "GShader Error");
            glAttachShader(_id, gs);
            glDeleteShader(gs);
        }

        if (fLength != 0)
        {
            uint fs = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fs, 1, &fString, (iint*)&fLength);
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

    const std::vector<PassAttr> & GetPassAttr() const
    {
        return _passAttrs;
    }

    void UsePass(uint i) const
    {
        ASSERT_LOG(i < _passAttrs.size(), "Out Range. {0}, {1}", _passAttrs.size(), i);
        const auto & attr  = _passAttrs.at(i);
        BindUniformSubProgram(attr.mPassName);
    }

    void Use() const
    {
        glUseProgram(_id);
    }

    void BindUniformNumber(const char * const key, iint val) const { glUniform1i(glGetUniformLocation(_id, key), val); }
    void BindUniformNumber(const char * const key, uint val) const { glUniform1i(glGetUniformLocation(_id, key), val); }
    void BindUniformNumber(const char * const key, float val) const { glUniform1f(glGetUniformLocation(_id, key), val); }
    void BindUniformNumber(const char * const key, double val) const { glUniform1f(glGetUniformLocation(_id, key), static_cast<float>(val)); }
    void BindUniformVector(const char * const key, const glm::vec3 & val) const { glUniform3f(glGetUniformLocation(_id, key), val.x, val.y, val.z); }
    void BindUniformVector(const char * const key, const glm::vec4 & val) const { glUniform4f(glGetUniformLocation(_id, key), val.x, val.y, val.z, val.w); }
    void BindUniformMatrix(const char * const key, const glm::mat3 & val) const { glUniformMatrix3fv(glGetUniformLocation(_id, key), 1, GL_FALSE, &val[0][0]); }
    void BindUniformMatrix(const char * const key, const glm::mat4 & val) const {  glUniformMatrix4fv(glGetUniformLocation(_id, key), 1, GL_FALSE, &val[0][0]); }

    void BindUniformTex2D(const char * const key, const uint val, iint pos) const
    {
        glActiveTexture(GL_TEXTURE0 + pos);
        glBindTexture(MMC_TEXTURE_2D, val);
        if (key != nullptr) { glUniform1i(glGetUniformLocation(_id, key), pos); }
    }

    void BindUniformTex3D(const char * const key, const uint val, iint pos) const
    {
        glActiveTexture(GL_TEXTURE0 + pos);
        glBindTexture(MMC_TEXTURE_3D, val);
        if (key != nullptr) { glUniform1i(glGetUniformLocation(_id, key), pos); }
    }

    void BindUniformSubProgram(const char * const val) const
    {
        auto vIndex = glGetSubroutineIndex(_id, GL_VERTEX_SHADER, val);
        auto gIndex = glGetSubroutineIndex(_id, GL_GEOMETRY_SHADER, val);
        auto fIndex = glGetSubroutineIndex(_id, GL_FRAGMENT_SHADER, val);
        if (vIndex != GL_INVALID_INDEX) { glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &vIndex); }
        if (gIndex != GL_INVALID_INDEX) { glUniformSubroutinesuiv(GL_GEOMETRY_SHADER, 1, &gIndex); }
        if (fIndex != GL_INVALID_INDEX) { glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &fIndex); }
    }

    uint GetID() const
    {
        return _id;
    }

private:
    void AssertPass(uint shaderID, const std::string & errorTxt) const
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