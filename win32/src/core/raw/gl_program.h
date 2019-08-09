#pragma once

#include "gl_res.h"
#include "gl_texture2d.h"

#define MMC_TEXTURE_2D  GL_TEXTURE_2D
#define MMC_TEXTURE_3D  GL_TEXTURE_CUBE_MAP

class GLProgramParam {
public:
    struct Value {
        Value() = default;
        Value(      Value && value) = default;
        Value(const Value &  value) = default;

        bool operator=(Value && value)
        {
            mKey = std::move(value.mKey);
            mVal = std::move(value.mVal);
        }

        bool operator=(const Value & value)
        {
            mKey = value.mKey;
            mVal = value.mVal;
        }

        template <class T>
        Value(const std::string & key, const T & val)
            : mKey(key), mVal(val)
        { }

        std::string mKey;
        std::any    mVal;
    };
    std::vector<Value> mValues;
};

class GLProgram : public GLRes {
public:
    struct Pass {
        //  面剔除
        iint        mCullFace;
        //  混合
        iint        mBlendSrc;
        iint        mBlendDst;
        //  深度测试
        iint        mDepthFunc;         //  深度测试函数
        short       mDepthTest;         //  开启深度测试
        short       mDepthWrite;        //  开启深度写入
        //  模板测试
        iint        mStencilOpFail;	    //	模板测试失败
        iint        mStencilOpZFail;	//	深度测试失败
        iint        mStencilOpZPass;	//	深度测试通过
        iint		mStencilFunc;	    //	模板测试函数
        iint        mStencilMask;       //  模板测试值
        iint        mStencilRef;        //  模板测试值
        //  渲染
        uint        mDrawType;			//	绘制类型
        uint        mFragType;			//	片段类型
        uint        mRenderType;        //  渲染类型
        uint        mRenderQueue;       //  渲染通道
        uint        mID;                //  ID

        Pass() { memset(this, 0, sizeof(Pass)); }
    };
public:
    GLProgram(): _useID(GL_INVALID_INDEX)
    { }

    ~GLProgram()
    { 
        for (auto & pass : _passs)
        {
            glDeleteProgram(pass.mID);
        }
    }

    const std::vector<Pass> & GetPasss() const
    {
        return _passs;
    }

    const Pass & GetPass(uint i) const
    {
        return _passs.at(i);
    }

    uint GetUseID() const
    {
        return _useID;
    }

    void AddPass(const Pass & pass, 
                 const char * vString, uint vLength,
                 const char * gString, uint gLength,
                 const char * fString, uint fLength)
    {
        _passs.push_back(pass);
        _passs.back().mID = glCreateProgram();

        if (vLength != 0)
        {
            uint vs = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vs, 1, &vString, (iint*)&vLength);
            glCompileShader(vs);
            AssertPass(vs, "Pass VShader Error.");
            glAttachShader(_passs.back().mID, vs);
            glDeleteShader(vs);
        }

        if (gLength != 0)
        {
            uint gs = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(gs, 1, &gString, (iint*)&gLength);
            glCompileShader(gs);
            AssertPass(gs, "Pass GShader Error.");
            glAttachShader(_passs.back().mID, gs);
            glDeleteShader(gs);
        }

        if (fLength != 0)
        {
            uint fs = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fs, 1, &fString, (iint*)&fLength);
            glCompileShader(fs);
            AssertPass(fs, "Pass FShader Error.");
            glAttachShader(_passs.back().mID, fs);
            glDeleteShader(fs);
        }

        iint ret;
        glLinkProgram( _passs.back().mID);
        glGetProgramiv(_passs.back().mID, GL_LINK_STATUS, &ret);
        ASSERT_LOG(ret != 0, "Pass GLProgram Error.");
    }

    bool UsePass(uint i, bool force = false) const
    {
        const auto & pass = _passs.at(i);
        if (pass.mID == _useID && !force)
        {
            return false;
        }

        if (pass.mCullFace != 0)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(pass.mCullFace);
        }
        else
        {
            glDisable(GL_CULL_FACE);
        }

        if (pass.mBlendSrc != 0 && pass.mBlendDst != 0)
        {
            glEnable(GL_BLEND);
            glBlendFunc(pass.mBlendSrc, pass.mBlendDst);
        }
        else
        {
            glDisable(GL_BLEND);
        }

        if (pass.mDepthTest)
        {
            glEnable(GL_DEPTH_TEST);
            if (pass.mDepthWrite)
            {
                glDepthMask(GL_TRUE);
            }
            else
            {
                glDepthMask(GL_FALSE);
            }
            ASSERT_LOG(pass.mDepthFunc != 0, "pass.mDepthFunc Must Not 0.");
            glDepthFunc(pass.mDepthFunc);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }
        
        if (pass.mStencilOpFail != 0 && pass.mStencilOpZFail != 0 && pass.mStencilOpZPass != 0)
        {
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xFF);
            glStencilFunc(pass.mStencilFunc, pass.mStencilRef, pass.mStencilMask);
            glStencilOp(pass.mStencilOpFail, pass.mStencilOpZFail, pass.mStencilOpZPass);
        }
        else
        {
            glDisable(GL_STENCIL_TEST);
        }
        glUseProgram(pass.mID);
        _useID      = pass.mID;
        return true;
    }

    void BindUniformNumber(const char * const key, iint val) const { glUniform1i(glGetUniformLocation(_useID, key), val); }
    void BindUniformNumber(const char * const key, uint val) const { glUniform1i(glGetUniformLocation(_useID, key), val); }
    void BindUniformNumber(const char * const key, float val) const { glUniform1f(glGetUniformLocation(_useID, key), val); }
    void BindUniformNumber(const char * const key, double val) const { glUniform1f(glGetUniformLocation(_useID, key), static_cast<float>(val)); }
    void BindUniformVector(const char * const key, const glm::vec2 & val) const { glUniform2f(glGetUniformLocation(_useID, key), val.x, val.y); }
    void BindUniformVector(const char * const key, const glm::vec3 & val) const { glUniform3f(glGetUniformLocation(_useID, key), val.x, val.y, val.z); }
    void BindUniformVector(const char * const key, const glm::vec4 & val) const { glUniform4f(glGetUniformLocation(_useID, key), val.x, val.y, val.z, val.w); }
    void BindUniformMatrix(const char * const key, const glm::mat3 & val) const { glUniformMatrix3fv(glGetUniformLocation(_useID, key), 1, GL_FALSE, &val[0][0]); }
    void BindUniformMatrix(const char * const key, const glm::mat4 & val) const {  glUniformMatrix4fv(glGetUniformLocation(_useID, key), 1, GL_FALSE, &val[0][0]); }

    void BindUniformTex2D(const char * const key, const uint val, iint pos) const
    {
        glActiveTexture(GL_TEXTURE0 + pos);
        glBindTexture(MMC_TEXTURE_2D, 0);
        glBindTexture(MMC_TEXTURE_3D, 0);
        glBindTexture(MMC_TEXTURE_2D, val);
        if (key != nullptr) { glUniform1i(glGetUniformLocation(_useID, key), pos); }
    }

    void BindUniformTex3D(const char * const key, const uint val, iint pos) const
    {
        glActiveTexture(GL_TEXTURE0 + pos);
        glBindTexture(MMC_TEXTURE_2D, 0);
        glBindTexture(MMC_TEXTURE_3D, 0);
        glBindTexture(MMC_TEXTURE_3D, val);
        if (key != nullptr) { glUniform1i(glGetUniformLocation(_useID, key), pos); }
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
    std::vector<Pass> _passs;
    mutable uint      _useID;
};