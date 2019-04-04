#pragma once

#include "asset.h"
#include "../render/render_type.h"

class Bitmap;
class Texture;
class BitmapCube;

class Shader : public Asset {
public:
    struct Pass {
        //  面剔除
        bool    bCullFace;          //  开启面剔除
        int     vCullFace;
        //  混合
        bool    bBlend;             //  开启混合
        int     vBlendSrc;
        int     vBlendDst;
        //  深度测试
        bool    bDepthTest;         //  开启深度测试
        bool    bDepthWrite;        //  开启深度写入
        //  模板测试
        bool    bStencilTest;       //  开启模板测试
        int     vStencilOpFail;
        int     vStencilOpZFail;
        int     vStencilOpZPass;
        int     vStencilMask;       //  模板测试值
        int     vStencilRef;        //  模板测试值
        //  渲染
        int     mRenderType;        //  渲染类型
        int     mRenderQueue;       //  渲染通道
        GLuint  GLID;

        Pass() : GLID(0), bCullFace(false), bBlend(false), bDepthTest(false), bStencilTest(false)
        { }
    };
public:
    ~Shader();

    bool IsEmpty() const
    {
        return _passs.empty();
    }

    const Pass & GetPass(size_t idx) const
    {
        return _passs.at(idx);
    }

    bool AddPass(
        const Pass & pass,
        const std::string & vs,
        const std::string & fs,
        const std::string & gs);

    void SetUniform(size_t idx, int val);
    void SetUniform(size_t idx, float val);
    void SetUniform(size_t idx, double val);
    void SetUniform(size_t idx, const glm::vec3 & val);
    void SetUniform(size_t idx, const glm::vec4 & val);
    void SetUniform(size_t idx, const glm::mat3 & val);
    void SetUniform(size_t idx, const glm::mat4 & val);
    void SetUniform(size_t idx, const Bitmap * val, size_t pos);
    void SetUniform(size_t idx, const Texture & val, size_t pos);
    void SetUniform(size_t idx, const BitmapCube * val, size_t pos);

    void SetUniform(size_t pass, const std::string & key, int val);
    void SetUniform(size_t pass, const std::string & key, float val);
    void SetUniform(size_t pass, const std::string & key, double val);
    void SetUniform(size_t pass, const std::string & key, const glm::vec3 & val);
    void SetUniform(size_t pass, const std::string & key, const glm::vec4 & val);
    void SetUniform(size_t pass, const std::string & key, const glm::mat3 & val);
    void SetUniform(size_t pass, const std::string & key, const glm::mat4 & val);
    void SetUniform(size_t pass, const std::string & key, const Bitmap * val, size_t pos);
    void SetUniform(size_t pass, const std::string & key, const Texture & val, size_t pos);
    void SetUniform(size_t pass, const std::string & key, const BitmapCube * val, size_t pos);

private:
    GLuint AddPass(const char * vs, const char * fs, const char * gs);
    void CheckPass(GLuint GLID, const std::string & string);

private:
    std::vector<Pass> _passs;
};