#pragma once

#include "gl_res.h"

class GLMesh: public GLRes {
public:
	struct Vertex {
		glm::vec3 v;
		glm::vec3 n;
        glm::vec4 c;
        glm::vec2 uv;
		glm::vec3 tan;
		glm::vec3 bitan;

        enum EnableEnum {
            kV = 0x1,
            kN = 0x2,
            kC = 0x4,
            kUV = 0x8,
            kTAN = 0x10,
            kBITAN = 0x20,
        };
	};

    GLMesh(): _vao(0), _vbo(0), _ebo(0), _vCount(0), _eCount(0)
    { }

    ~GLMesh()
    {
        glDeleteBuffers(1, &_vbo);
        glDeleteBuffers(1, &_ebo);
        glDeleteVertexArrays(1, &_vao);
    }

    //  完全填充
    void Init(const std::vector<Vertex> & vertexs, const std::vector<uint> & indexs, uint enabled, uint vUsage = GL_STATIC_DRAW, uint eUsage = GL_STATIC_DRAW)
    {
        _vCount = static_cast<uint>(indexs.size());
        _eCount = static_cast<uint>(vertexs.size());

        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);

        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertexs.size(), vertexs.data(), vUsage);

        glGenBuffers(1, &_ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indexs.size(), indexs.data(), eUsage);

        auto idx = 0;
        glVertexAttribPointer(idx, decltype(Vertex::v)::length(), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, v));
        glEnableVertexAttribArray(idx++);
        glVertexAttribPointer(idx, decltype(Vertex::n)::length(), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, n));
        glEnableVertexAttribArray(idx++);
        glVertexAttribPointer(idx, decltype(Vertex::c)::length(), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, c));
        glEnableVertexAttribArray(idx++);
        glVertexAttribPointer(idx, decltype(Vertex::uv)::length(), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
        glEnableVertexAttribArray(idx++);
        glVertexAttribPointer(idx, decltype(Vertex::tan)::length(), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, tan));
        glEnableVertexAttribArray(idx++);
        glVertexAttribPointer(idx, decltype(Vertex::bitan)::length(), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, bitan));
        glEnableVertexAttribArray(idx++);

        glBindVertexArray(0);
    }

private:
	uint _vao, _vbo, _ebo;
    uint _vCount, _eCount;
	

	//	顶点数据
	//		顶点坐标，纹理坐标，法线，切线，副切线，颜色
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
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, c));
        glEnableVertexAttribArray(5);

		glBindVertexArray(0);
		return mesh;
	}

	//	索引数据
	//		顶点坐标，纹理坐标，法线，切线，副切线，颜色
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
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, c));
        glEnableVertexAttribArray(5);

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

    //	索引数据
    //		顶点坐标，颜色
    static Mesh CreateVC(const std::vector<Vertex> & vertexs, const std::vector<uint> & indexs)
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
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, c));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
        return mesh;
    }

	static void DeleteRef(Mesh & mesh)
	{
		glDeleteBuffers(1, &mesh.mEBO);
		glDeleteBuffers(1, &mesh.mVBO);
		glDeleteVertexArrays(1, &mesh.mVAO);
		mesh.mVtxCount = mesh.mIdxCount = mesh.mVAO = mesh.mVBO = mesh.mEBO = 0;
	}

    static void DeletePtr(Mesh * mesh)
    {
        DeleteRef(*mesh);
    }
};
