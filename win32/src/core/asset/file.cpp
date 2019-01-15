#include "file.h"
#include "../mmc.h"
#include "asset_core.h"
#include "../tools/debug_tool.h"
#include "../tools/string_tool.h"

Mesh * File::LoadMesh(const std::string & url)
{
	CHECK_RET(!mmc::mAssetCore.IsReg(url), mmc::mAssetCore.Get<Mesh>(url));

	std::string line;
	std::ifstream ifile(url);
	std::vector<glm::vec3> vs;
	std::vector<glm::vec3> fs;
	std::vector<glm::vec3> vns;
	std::vector<glm::vec2> vts;
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
			assert(split.size() == 3);
			for (const auto & view : split)
			{
				glm::vec3 f;
				auto fdata = string_tool::Split(view, "/");
				f.x = (float)std::atof(std::string(fdata.at(0)).c_str());
				f.y = (float)std::atof(std::string(fdata.at(1)).c_str());
				f.z = (float)std::atof(std::string(fdata.at(2)).c_str());
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
		else if (key == "vn")
		{
			glm::vec3 n;
			auto split = string_tool::Split(view.substr(key.size() + 1), " ");
			n.x = (float)std::atof(std::string(split.at(0)).c_str());
			n.y = (float)std::atof(std::string(split.at(1)).c_str());
			n.z = (float)std::atof(std::string(split.at(2)).c_str());
			vns.push_back(n);
		}
	}
	std::vector<Mesh::Vertex> vertexs;
	for (const auto & f : fs)
	{
		Mesh::Vertex vertex;
		vertex.v.x = vs.at((size_t)f.x - 1).x;
		vertex.v.y = vs.at((size_t)f.x - 1).y;
		vertex.v.z = vs.at((size_t)f.x - 1).z;
		vertex.uv.u = vts.at((size_t)f.y - 1).x;
		vertex.uv.v = vts.at((size_t)f.y - 1).y;
		vertex.n.x = vns.at((size_t)f.z - 1).x;
		vertex.n.y = vns.at((size_t)f.z - 1).y;
		vertex.n.z = vns.at((size_t)f.z - 1).z;
		vertexs.push_back(vertex);
	}
	auto mesh = new Mesh(std::move(vertexs));
	mmc::mAssetCore.Reg(url, mesh);
	return mesh;
}

Shader * File::LoadShader(const std::string & url)
{
	CHECK_RET(!mmc::mAssetCore.IsReg(url), mmc::mAssetCore.Get<Shader>(url));

	auto vs = url + ".vs";
	auto fs = url + ".fs";
	std::ifstream vfile(vs);
	std::ifstream ffile(fs);
	CHECK_RET(vfile && ffile, nullptr);
	std::stringstream vss, fss;
	vss << vfile.rdbuf();
	fss << ffile.rdbuf();
	vfile.close();
	ffile.close();
	auto shader = new Shader(vss.str(), fss.str());
	mmc::mAssetCore.Reg(url, shader);
	return shader;
}

Bitmap * File::LoadBitmap(const std::string & url, int format)
{
	CHECK_RET(!mmc::mAssetCore.IsReg(url), mmc::mAssetCore.Get<Bitmap>(url));
	
	stbi_set_flip_vertically_on_load(true);
	auto w = 0, h = 0, c = 0;
	auto buffer = stbi_load(url.c_str(), &w, &h, &c, 0);
	CHECK_RET(buffer != nullptr, nullptr);
	Bitmap::Data data;
	data.format = format;
	data.url = url;
	data.w = w;
	data.h = h;
	auto bitmap = new Bitmap(std::move(data), buffer);
	mmc::mAssetCore.Reg(url, bitmap);
	stbi_image_free(buffer);
	return bitmap;
}

Texture File::LoadTexture(const std::string & url, int format)
{
	return Texture(File::LoadBitmap(url, format));
}

Material * File::LoadMaterial(const std::string & url)
{
	CHECK_RET(!mmc::mAssetCore.IsReg(url), mmc::mAssetCore.Get<Material>(url));

	std::ifstream ifile(url);
	if (ifile)
	{
		Material::Data data;

		//	环境光分量
		std::string line;
		std::getline(ifile, line);
		auto split = string_tool::Split(line, " ");
		data.mAmbient.x = (float)std::atof(std::string(split.at(0)).c_str());
		data.mAmbient.y = (float)std::atof(std::string(split.at(1)).c_str());
		data.mAmbient.z = (float)std::atof(std::string(split.at(2)).c_str());

		//	漫反射分量
		std::getline(ifile, line);
		split = string_tool::Split(line, " ");
		data.mDiffuse.x = (float)std::atof(std::string(split.at(0)).c_str());
		data.mDiffuse.y = (float)std::atof(std::string(split.at(1)).c_str());
		data.mDiffuse.z = (float)std::atof(std::string(split.at(2)).c_str());

		//	镜面反射分量
		std::getline(ifile, line);
		split = string_tool::Split(line, " ");
		data.mSpecular.x = (float)std::atof(std::string(split.at(0)).c_str());
		data.mSpecular.y = (float)std::atof(std::string(split.at(1)).c_str());
		data.mSpecular.z = (float)std::atof(std::string(split.at(2)).c_str());

		std::getline(ifile, line);
		data.mShininess = (float)std::atof(line.c_str());

		auto material = new Material(data);
		mmc::mAssetCore.Reg(url, material);
		return material;
	}
	return nullptr;
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
