#pragma once

#include "gl_res.h"

#define MMC_TEXTURE_2D  GL_TEXTURE_2D
#define MMC_TEXTURE_3D  GL_TEXTURE_CUBE_MAP

class GLProgram : public GLRes {
public:
    struct PassAttr {
        //  ���޳�
        int     vCullFace;
        //  ���
        int     vBlendSrc;
        int     vBlendDst;
        //  ��Ȳ���
        bool    bDepthTest;         //  ������Ȳ���
        bool    bDepthWrite;        //  �������д��
        //  ģ�����
        int     vStencilOpFail;		//	ģ�����ʧ��
        int     vStencilOpZFail;	//	��Ȳ���ʧ��
        int     vStencilOpZPass;	//	��Ȳ���ͨ��
        int		vStencilFunc;		//	ģ����Ժ���
        int     vStencilMask;       //  ģ�����ֵ
        int     vStencilRef;        //  ģ�����ֵ
        //  ��Ⱦ
        uint    vDrawType;			//	��������
        uint    vRenderType;        //  ��Ⱦ����
        uint    vRenderQueue;       //  ��Ⱦͨ��
        char    mPassName[12];      //  Pass����
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

    void UsePass(uint i)
    {
        ASSERT_LOG(i < _passAttrs.size(), "Out Range. {0}, {1}", _passAttrs.size(), i);
        const auto & attr  = _passAttrs.at(i);
        BindUniformSubProgram(attr.mPassName);
    }

    void Use()
    {
        glUseProgram(_id);
    }

    void BindUniformNumber(const char * const key, iint val) { glUniform1i(glGetUniformLocation(_id, key), val); }
    void BindUniformNumber(const char * const key, uint val) { glUniform1i(glGetUniformLocation(_id, key), val); }
    void BindUniformNumber(const char * const key, float val) { glUniform1f(glGetUniformLocation(_id, key), val); }
    void BindUniformNumber(const char * const key, double val) { glUniform1f(glGetUniformLocation(_id, key), static_cast<float>(val)); }
    void BindUniformVector(const char * const key, const glm::vec3 & val) { glUniform3f(glGetUniformLocation(_id, key), val.x, val.y, val.z); }
    void BindUniformVector(const char * const key, const glm::vec4 & val) { glUniform4f(glGetUniformLocation(_id, key), val.x, val.y, val.z, val.w); }
    void BindUniformMatrix(const char * const key, const glm::mat3 & val) { glUniformMatrix3fv(glGetUniformLocation(_id, key), 1, GL_FALSE, &val[0][0]); }
    void BindUniformMatrix(const char * const key, const glm::mat4 & val) {  glUniformMatrix4fv(glGetUniformLocation(_id, key), 1, GL_FALSE, &val[0][0]); }

    void BindUniformTex2D(const char * const key, const uint val, iint pos)
    {
        glActiveTexture(GL_TEXTURE0 + pos);
        glBindTexture(MMC_TEXTURE_2D, val);
        if (key != nullptr) { glUniform1i(glGetUniformLocation(_id, key), pos); }
    }

    void BindUniformTex3D(const char * const key, const uint val, iint pos)
    {
        glActiveTexture(GL_TEXTURE0 + pos);
        glBindTexture(MMC_TEXTURE_3D, val);
        if (key != nullptr) { glUniform1i(glGetUniformLocation(_id, key), pos); }
    }

    void BindUniformSubProgram(const char * const val)
    {
        auto vIndex = glGetSubroutineIndex(_id, GL_VERTEX_SHADER, val);
        auto gIndex = glGetSubroutineIndex(_id, GL_GEOMETRY_SHADER, val);
        auto fIndex = glGetSubroutineIndex(_id, GL_FRAGMENT_SHADER, val);
        glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &vIndex);
        glUniformSubroutinesuiv(GL_GEOMETRY_SHADER, 1, &gIndex);
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &fIndex);
    }

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