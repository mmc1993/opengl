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
	};

public:
	Mesh(std::vector<Vertex> && vertexs) 
		: _vertexs(std::move(vertexs)), _vao(0), _vbo(0)
	{
		glGenVertexArrays(1, &_vao);
		glBindVertexArray(_vao);
		glGenBuffers(1, &_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex), _vertexs.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)(size_t)sizeof(glm::vec3));
		glEnableVertexAttribArray(1);
		glBindVertexArray(0);
	}

	virtual ~Mesh()
	{
		glDeleteBuffers(1, &_vbo);
		_vbo = 0;
		glDeleteVertexArrays(1, &_vao);
		_vao = 0;
	}

	const std::vector<Vertex> & GetVertexs() const
	{
		return _vertexs;
	}

	void Bind() 
	{
		assert(_vao != 0);
		assert(_vbo != 0);
		glBindVertexArray(_vao);
	}

private:
	GLuint _vao;
	GLuint _vbo;
	std::vector<Vertex> _vertexs;
};