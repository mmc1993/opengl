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
