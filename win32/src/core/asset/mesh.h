#pragma once

#include "asset.h"

class Mesh: public Asset {
public:
	struct Vertex {
		struct {
			float u;
			float v;
		} uv;
		glm::vec4 v;
	};

public:
	Mesh(std::vector<Vertex> && vertexs) 
		: _vertexs(std::move(vertexs))
	{
	}

	virtual ~Mesh()
	{
	}

	const std::vector<Vertex> & GetVertexs() const
	{
		return _vertexs;
	}

private:
	std::vector<Vertex> _vertexs;
};