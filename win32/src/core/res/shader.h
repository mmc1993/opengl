#pragma once

#include "res.h"
#include "../render/render_enum.h"

class Pass {
public:
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
    int     vStencilOpFail;		//	模板测试失败
    int     vStencilOpZFail;	//	深度测试失败
    int     vStencilOpZPass;	//	深度测试通过
    int		vStencilFunc;		//	模板测试函数
    int     vStencilMask;       //  模板测试值
    int     vStencilRef;        //  模板测试值
                                //  渲染
    DrawTypeEnum		mDrawType;			//	draw类型
    RenderTypeEnum		mRenderType;        //  渲染类型
    RenderQueueEnum     mRenderQueue;       //  渲染通道
                                            //	Shader ID
    uint  GLID;

    Pass() : GLID(0), bCullFace(false), bBlend(false), bDepthTest(false), bStencilTest(false)
    { }
};

class Bitmap;
class BitmapCube;

class Shader : public Res {
public:
    static void SetUniform(uint GLID, const std::string & key, iint val);
    static void SetUniform(uint GLID, const std::string & key, uint val);
	static void SetUniform(uint GLID, const std::string & key, float val);
	static void SetUniform(uint GLID, const std::string & key, double val);
	static void SetUniform(uint GLID, const std::string & key, const glm::vec3 & val);
	static void SetUniform(uint GLID, const std::string & key, const glm::vec4 & val);
	static void SetUniform(uint GLID, const std::string & key, const glm::mat3 & val);
	static void SetUniform(uint GLID, const std::string & key, const glm::mat4 & val);
    static void SetTexture2D(uint GLID, const std::string & key, const uint val, iint pos);
    static void SetTexture3D(uint GLID, const std::string & key, const uint val, iint pos);
    static void SetUniform(uint GLID, const std::string & key, const Bitmap * val, iint pos);
	static void SetUniform(uint GLID, const std::string & key, const BitmapCube * val, iint pos);

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

    const std::vector<Pass> & GetPasss() const
    {
        return _passs;
    }

    bool AddPass(
        const Pass & pass,
        const std::string & vs,
        const std::string & fs,
        const std::string & gs);

private:
    uint AddPass(const char * vs, 
				   const char * fs, 
				   const char * gs);

    void CheckPass(uint GLID, const std::string & string);

private:
    std::vector<Pass> _passs;
};