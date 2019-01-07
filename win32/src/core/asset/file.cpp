#include "file.h"
#include "../tools/debug_tool.h"
#include "../tools/string_tool.h"

std::vector<Mesh::Vertex>&& File::LoadMesh(const std::string & fname)
{
	std::vector<glm::vec3> vs;
	std::vector<glm::vec2> fs;
	std::vector<glm::vec2> vts;
	std::vector<Mesh::Vertex> vertexs;

	std::string line;
	std::ifstream ifile(fname);
	while (std::getline(ifile, line))
	{
		std::string_view view(line);
		auto key = FindSubStrUntil(line, " ");
		if (key == "v")
		{
			glm::vec3 v;
			auto split = string_tool::Split(view.substr(key.size() + 1), " ");
			v.x = (float)std::atof(std::string(split.at(0)).c_str());
			v.y = (float)std::atof(std::string(split.at(1)).c_str());
			v.z = (float)std::atof(std::string(split.at(2)).c_str());
			vs.push_back(v);
		}
		else if (key == "f")
		{
			auto split = string_tool::Split(view.substr(key.size() + 1), " ");
			for (const auto & view : split)
			{
				glm::vec2 f;
				auto fdata = string_tool::Split(view, "/");
				f.x = (float)std::atof(std::string(fdata.at(0)).c_str());
				f.y = (float)std::atof(std::string(fdata.at(1)).c_str());
				fs.push_back(f);
			}
		}
		else if (key == "vt")
		{
			glm::vec2 vt;
			auto split = string_tool::Split(view.substr(key.size() + 1), " ");
			vt.x = (float)std::atof(std::string(split.at(0)).c_str());
			vt.y = (float)std::atof(std::string(split.at(1)).c_str());
			vts.push_back(vt);
		}
	}
	for (const auto & f : fs)
	{
		Mesh::Vertex vertex;
		vertex.v.x = vs.at((size_t)f.x).x;
		vertex.v.y = vs.at((size_t)f.x).y;
		vertex.v.z = vs.at((size_t)f.x).z;
		vertex.v.w = 1.0f;
		vertex.uv.u = vts.at((size_t)f.y).x;
		vertex.uv.v = vts.at((size_t)f.y).y;
		vertexs.push_back(vertex);
	}
	return std::move(vertexs);
}

Material::Data && File::LoadMaterial(const std::string & fname)
{
	Material::Data data;
	std::ifstream ifile(fname);
	if (ifile)
	{
		std::string meshURL;
		CHECK_RET(std::getline(ifile, meshURL), std::move(data));
		std::string normalURL;
		CHECK_RET(std::getline(ifile, normalURL), std::move(data));
		std::string textureURL;
		CHECK_RET(std::getline(ifile, textureURL), std::move(data));
		std::string shadervURL;
		CHECK_RET(std::getline(ifile, shadervURL), std::move(data));
		std::string shaderfURL;
		CHECK_RET(std::getline(ifile, shaderfURL), std::move(data));

		//	TODO mmc
		//	暂时不考虑重用，第一版本简单粗暴实现
		auto mesh = new Mesh(File::LoadMesh(meshURL));
		
	}
	return Material::Data && ();
}

std::string_view File::FindSubStrUntil(
	const std::string_view & str,
	const std::string_view & end, 
	size_t pos)
{
	auto idx = str.find(end, pos);
	if (idx == std::string::npos)
	{
		return std::string_view();
	}
	return str.substr(pos, idx - pos);
}
