#pragma once

#include "asset.h"

class Mesh: public Asset {
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
	};

public:
	Mesh(
		std::vector<Vertex> && vertexs,
		std::vector<std::uint32_t> && indices)
		: _vertexs(std::move(vertexs))
		, _indices(std::move(indices))
		, _vao(0), _vbo(0), _ebo(0)
	{
		glGenVertexArrays(1, &_vao);
		glBindVertexArray(_vao);
		
		glGenBuffers(1, &_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		glBufferData(GL_ARRAY_BUFFER, _vertexs.size() * sizeof(Vertex), _vertexs.data(), GL_STATIC_DRAW);
		
		glGenBuffers(1, &_ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(size_t), _indices.data(), GL_STATIC_DRAW);
		
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
	}

	virtual ~Mesh()
	{
		glDeleteBuffers(1, &_vbo);
		glDeleteBuffers(1, &_ebo);
		glDeleteVertexArrays(1, &_vao);
		_vbo = _vao = _ebo = 0;
	}

	const std::vector<Vertex> & GetVertexs() const
	{
		return _vertexs;
	}

	const std::vector<uint32_t> & GetIndices() const
	{
		return _indices;
	}

	GLuint GetGLID() const
	{
		assert(_vao != 0);
		assert(_vbo != 0);
		assert(_ebo != 0);
		return _vao;
	}

	size_t GetIdxCount() const
	{
		return _indices.size();
	}

	size_t GetVerCount() const
	{
		return _vertexs.size();
	}

private:
	GLuint _vao;
	GLuint _vbo;
	GLuint _ebo;
	std::vector<Vertex> _vertexs;
	std::vector<std::uint32_t> _indices;
};