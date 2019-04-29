#pragma once

#include "res.h"

class Mesh {
public:
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

	uint mVAO, mVBO, mEBO;
	
	uint mVtxCount, mIdxCount;

	Mesh() 
		: mVAO(0), mVBO(0), mEBO(0)
		, mVtxCount(0), mIdxCount(0)
	{ }

	//	顶点数据
	//		顶点坐标，纹理坐标，法线，切线，副切线
	static Mesh Create(const std::vector<Vertex> & vertexs)
	{
		Mesh mesh;
		mesh.mVtxCount = static_cast<uint>(vertexs.size());
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
	static Mesh Create(const std::vector<Vertex> & vertexs, const std::vector<uint> & indexs)
	{
		Mesh mesh;
		mesh.mIdxCount = static_cast<uint>(indexs.size());
		mesh.mVtxCount = static_cast<uint>(vertexs.size());
		glGenVertexArrays(1, &mesh.mVAO);
		glBindVertexArray(mesh.mVAO);

		glGenBuffers(1, &mesh.mVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.mVBO);
		glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(Vertex), vertexs.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &mesh.mEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexs.size() * sizeof(uint), indexs.data(), GL_STATIC_DRAW);

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
	static Mesh CreateVTN(const std::vector<Vertex> & vertexs, const std::vector<uint> & indexs)
	{
		Mesh mesh;
		mesh.mIdxCount = static_cast<uint>(indexs.size());
		mesh.mVtxCount = static_cast<uint>(vertexs.size());
		glGenVertexArrays(1, &mesh.mVAO);
		glBindVertexArray(mesh.mVAO);

		glGenBuffers(1, &mesh.mVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.mVBO);
		glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(Vertex), vertexs.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &mesh.mEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexs.size() * sizeof(uint), indexs.data(), GL_STATIC_DRAW);

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
	static Mesh CreateVT(const std::vector<Vertex> & vertexs, const std::vector<uint> & indexs)
	{
		Mesh mesh;
		mesh.mIdxCount = static_cast<uint>(indexs.size());
		mesh.mVtxCount = static_cast<uint>(vertexs.size());
		glGenVertexArrays(1, &mesh.mVAO);
		glBindVertexArray(mesh.mVAO);

		glGenBuffers(1, &mesh.mVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.mVBO);
		glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(Vertex), vertexs.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &mesh.mEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexs.size() * sizeof(uint), indexs.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, v));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		return mesh;
	}

	//	索引数据
	//		顶点坐标
	static Mesh CreateV(const std::vector<Vertex> & vertexs, const std::vector<uint> & indexs)
	{
		Mesh mesh;
		mesh.mIdxCount = static_cast<uint>(indexs.size());
		mesh.mVtxCount = static_cast<uint>(vertexs.size());
		glGenVertexArrays(1, &mesh.mVAO);
		glBindVertexArray(mesh.mVAO);

		glGenBuffers(1, &mesh.mVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.mVBO);
		glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(Vertex), vertexs.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &mesh.mEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexs.size() * sizeof(uint), indexs.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, v));
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
		return mesh;
	}

	static void Delete(Mesh & mesh)
	{
		glDeleteBuffers(1, &mesh.mEBO);
		glDeleteBuffers(1, &mesh.mVBO);
		glDeleteVertexArrays(1, &mesh.mVAO);
		mesh.mVtxCount = mesh.mIdxCount = mesh.mVAO = mesh.mVBO = mesh.mEBO = 0;
	}
};
