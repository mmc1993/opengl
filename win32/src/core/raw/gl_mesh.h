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
            kV_N_C_UV_TAN_BITAN = kV | kN | kC | kUV | kTAN | kBITAN,
        };

        Vertex()
        { }

        Vertex(const glm::vec3 & _v): v(_v)
        { }

        Vertex(const glm::vec3 & _v, const glm::vec3 & _n) : v(_v), n(_n)
        { }

        Vertex(const glm::vec3 & _v, const glm::vec3 & _n, const glm::vec4 & _c) : v(_v), n(_n), c(_c)
        { }

        static uint SizeOf(uint enabled)
        {
            uint size = 0;
            if (kV & enabled) { size += sizeof(Vertex::v); }
            if (kN & enabled) { size += sizeof(Vertex::n); }
            if (kC & enabled) { size += sizeof(Vertex::c); }
            if (kUV & enabled) { size += sizeof(Vertex::uv); }
            if (kTAN & enabled) { size += sizeof(Vertex::tan); }
            if (kBITAN & enabled) { size += sizeof(Vertex::bitan); }
            return size;
        }
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
        Init(vertexs.data(), (uint)vertexs.size(), indexs.data(), (uint)indexs.size(), enabled, vUsage, eUsage);
    }

    void Init(
        const Vertex * vertexs, const uint vertexLength, 
        const uint   * indexs,  const uint indexLength, 
        uint enabled, uint vUsage = GL_STATIC_DRAW, uint eUsage = GL_STATIC_DRAW)
    {
        //  根据启用的顶点属性, 计算出单个顶点的数据大小.
        const auto vertexSize = Vertex::SizeOf(enabled);

        _vCount = vertexLength;
        _eCount = indexLength;

        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);

        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, vertexSize * _vCount, vertexs, vUsage);

        glGenBuffers(1, &_ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * _eCount, indexs, eUsage);

        auto idx = 0;
        if (Vertex::kV & enabled)
        {
            glVertexAttribPointer(idx, decltype(Vertex::v)::length(), GL_FLOAT, GL_FALSE, vertexSize, (void *)offsetof(Vertex, v));
            glEnableVertexAttribArray(idx++);
        }
        if (Vertex::kN & enabled)
        {
            glVertexAttribPointer(idx, decltype(Vertex::n)::length(), GL_FLOAT, GL_FALSE, vertexSize, (void *)offsetof(Vertex, n));
            glEnableVertexAttribArray(idx++);
        }
        if (Vertex::kC & enabled)
        {
            glVertexAttribPointer(idx, decltype(Vertex::c)::length(), GL_FLOAT, GL_FALSE, vertexSize, (void *)offsetof(Vertex, c));
            glEnableVertexAttribArray(idx++);
        }
        if (Vertex::kUV & enabled)
        {
            glVertexAttribPointer(idx, decltype(Vertex::uv)::length(), GL_FLOAT, GL_FALSE, vertexSize, (void *)offsetof(Vertex, uv));
            glEnableVertexAttribArray(idx++);
        }
        if (Vertex::kTAN & enabled)
        {
            glVertexAttribPointer(idx, decltype(Vertex::tan)::length(), GL_FLOAT, GL_FALSE, vertexSize, (void *)offsetof(Vertex, tan));
            glEnableVertexAttribArray(idx++);
        }
        if (Vertex::kBITAN & enabled)
        {
            glVertexAttribPointer(idx, decltype(Vertex::bitan)::length(), GL_FLOAT, GL_FALSE, vertexSize, (void *)offsetof(Vertex, bitan));
            glEnableVertexAttribArray(idx++);
        }

        glBindVertexArray(0);
    }

    uint GetVAO() const { return _vao; }
    uint GetVBO() const { return _vbo; }
    uint GetEBO() const { return _ebo; }
    uint GetVCount() const { return _vCount; }
    uint GetECount() const { return _eCount; }

private:
	uint _vao, _vbo, _ebo;
    uint _vCount, _eCount;
};
