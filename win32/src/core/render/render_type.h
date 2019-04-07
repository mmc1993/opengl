#pragma once

#include "../include.h"

class Light;
class Camera;
class Shader;
class Bitmap;
class Texture;
class Material;
class BitmapCube;

enum RenderQueueEnum {
	kBACKGROUND,    //  底层绘制
	kGEOMETRIC,     //  常规绘制
	kOPACITY,       //  透明绘制
	kTOP,           //  顶层绘制
};

enum RenderTypeEnum {
	kSHADOW,        //  烘培阴影贴图
	kFORWARD,       //  正向渲染
	kDEFERRED,      //  延迟渲染
};

enum DrawTypeEnum {
	kINSTANCE,		//	实例draw
	kVERTEX,		//	顶点draw
	kINDEX,			//	索引draw
};


class RenderMatrix {
public:
	enum ModeType { kPROJ, kVIEW, kMODEL, };

public:
	RenderMatrix()
	{ }

	~RenderMatrix()
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
	std::stack<glm::mat4> & GetStack(ModeType mode)
	{
		return _RenderMatrixs.at((size_t)mode);
	}

	const std::stack<glm::mat4> & GetStack(ModeType mode) const
	{
		return const_cast<RenderMatrix *>(this)->GetStack(mode);
	}

private:
	std::array<std::stack<glm::mat4>, 3> _RenderMatrixs;
};

struct RenderMesh {

	struct Vertex {
		glm::vec3 v;
		struct {
			float u;
			float v;
		} uv; 
		glm::vec3 n;
		glm::vec3 tan;
		glm::vec3 bitan;

		Vertex(const glm::vec3 & _v, 
			   float _tx, float _ty, 
			   const glm::vec3 & _n, 
			   const glm::vec3 & _tan, 
			   const glm::vec3 & _bitan) : v(_v), uv{ _tx, _ty }, n(_n), tan(_tan), bitan(_bitan)
		{ }

		Vertex(const glm::vec3 & _v,
			   float _tx, float _ty,
			   const glm::vec3 & _n) : v(_v), uv{ _tx, _ty }, n(_n)
		{ }

		Vertex(const glm::vec3 & _v,
			   float _tx, float _ty) : v(_v), uv{ _tx, _ty }
		{ }

		Vertex(const glm::vec3 & _v) : v(_v)
		{ }

		Vertex() 
		{ }
	};

	GLuint mVAO, mVBO, mEBO;
	
	size_t mVtxCount, mIdxCount;

	RenderMesh() 
		: mVAO(0), mVBO(0), mEBO(0)
		, mVtxCount(0), mIdxCount(0)
	{ }

	//	顶点数据
	//		顶点坐标，纹理坐标，法线，切线，副切线
	static RenderMesh Create(const std::vector<Vertex> & vertexs)
	{
		RenderMesh mesh;
		mesh.mVtxCount = vertexs.size();
		glGenVertexArrays(1, &mesh.mVAO);
		glBindVertexArray(mesh.mVAO);

		glGenBuffers(1, &mesh.mVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.mVBO);
		glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(Vertex), vertexs.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, v));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, n));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, tan));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, bitan));
		glEnableVertexAttribArray(4);

		glBindVertexArray(0);
		return mesh;
	}

	//	索引数据
	//		顶点坐标，纹理坐标，法线，切线，副切线
	static RenderMesh Create(const std::vector<Vertex> & vertexs, const std::vector<GLuint> & indexs)
	{
		RenderMesh mesh;
		mesh.mIdxCount = indexs.size();
		mesh.mVtxCount = vertexs.size();
		glGenVertexArrays(1, &mesh.mVAO);
		glBindVertexArray(mesh.mVAO);

		glGenBuffers(1, &mesh.mVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.mVBO);
		glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(Vertex), vertexs.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &mesh.mEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexs.size() * sizeof(size_t), indexs.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, v));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, n));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, tan));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, bitan));
		glEnableVertexAttribArray(4);

		glBindVertexArray(0);
		return mesh;
	}

	//	索引数据
	//		顶点坐标，纹理坐标，法线
	static RenderMesh CreateVTN(const std::vector<Vertex> & vertexs, const std::vector<GLuint> & indexs)
	{
		RenderMesh mesh;
		mesh.mIdxCount = indexs.size();
		mesh.mVtxCount = vertexs.size();
		glGenVertexArrays(1, &mesh.mVAO);
		glBindVertexArray(mesh.mVAO);

		glGenBuffers(1, &mesh.mVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.mVBO);
		glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(Vertex), vertexs.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &mesh.mEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexs.size() * sizeof(size_t), indexs.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, v));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, n));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
		return mesh;
	}

	//	索引数据
	//		顶点坐标，纹理坐标
	static RenderMesh CreateVT(const std::vector<Vertex> & vertexs, const std::vector<GLuint> & indexs)
	{
		RenderMesh mesh;
		mesh.mIdxCount = indexs.size();
		mesh.mVtxCount = vertexs.size();
		glGenVertexArrays(1, &mesh.mVAO);
		glBindVertexArray(mesh.mVAO);

		glGenBuffers(1, &mesh.mVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.mVBO);
		glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(Vertex), vertexs.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &mesh.mEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexs.size() * sizeof(size_t), indexs.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, v));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		return mesh;
	}

	//	索引数据
	//		顶点坐标
	static RenderMesh CreateV(const std::vector<Vertex> & vertexs, const std::vector<GLuint> & indexs)
	{
		RenderMesh mesh;
		mesh.mIdxCount = indexs.size();
		mesh.mVtxCount = vertexs.size();
		glGenVertexArrays(1, &mesh.mVAO);
		glBindVertexArray(mesh.mVAO);

		glGenBuffers(1, &mesh.mVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.mVBO);
		glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(Vertex), vertexs.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &mesh.mEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexs.size() * sizeof(size_t), indexs.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, v));
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
		return mesh;
	}


	static void Delete(RenderMesh & mesh)
	{
		glDeleteBuffers(1, &mesh.mEBO);
		glDeleteBuffers(1, &mesh.mVBO);
		glDeleteVertexArrays(1, &mesh.mVAO);
		mesh.mVtxCount = mesh.mIdxCount = mesh.mVAO = mesh.mVBO = mesh.mEBO = 0;
	}
};


struct RenderPass {
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
    GLuint  GLID;

    RenderPass() : GLID(0), bCullFace(false), bBlend(false), bDepthTest(false), bStencilTest(false)
    { }
};

//  用于渲染的命令结构
struct RenderCommand {
    const RenderPass *  mPass;          //  绑定的Shader
    const RenderMesh *  mMeshs;         //  绑定的网格
	size_t              mMeshNum;       //  绑定的网格数量
	const Material *    mMaterials;     //  绑定的材质(材质与网格数量必须一致)
	glm::mat4           mTransform;     //  绑定的变换矩阵
    size_t              mCameraFlag;    //  绑定的相机标识
};

using RenderQueue = std::vector<RenderCommand>;