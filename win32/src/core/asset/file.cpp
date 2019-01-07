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
	std::vector<glm::vec2> fs;
	std::vector<glm::vec2> vts;
	std::vector<Mesh::Vertex> vertexs;
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

Bitmap * File::LoadBitmap(const std::string & url)
{
	CHECK_RET(!mmc::mAssetCore.IsReg(url), mmc::mAssetCore.Get<Bitmap>(url));
	
	stbi_set_flip_vertically_on_load(1);
	auto w = 0, h = 0, c = 0;
	auto buffer = stbi_load(url.c_str(), &w, &h, &c, 0);
	CHECK_RET(buffer != nullptr, nullptr);
	Bitmap::Data data;
	data.channel = c;
	data.url = url;
	data.w = w;
	data.h = h;
	auto bitmap = new Bitmap(std::move(data), buffer);
	mmc::mAssetCore.Reg(url, bitmap);
	stbi_image_free(buffer);
	return bitmap;
}

Texture File::LoadTexture(const std::string & url)
{
	return Texture(File::LoadBitmap(url));
}

Material * File::LoadMaterial(const std::string & url)
{
	CHECK_RET(mmc::mAssetCore.IsReg(url), mmc::mAssetCore.Get<Material>(url));

	std::ifstream ifile(url);
	if (ifile)
	{
		Material::Data data;
		CHECK_RET(std::getline(ifile, data.mMeshURL), nullptr);
		CHECK_RET(std::getline(ifile, data.mNormalURL), nullptr);
		CHECK_RET(std::getline(ifile, data.mTextureURL), nullptr);
		CHECK_RET(std::getline(ifile, data.mShaderURL), nullptr);
		data.mMesh = File::LoadMesh(data.mMeshURL);
		data.mShader = File::LoadShader(data.mShaderURL);
		data.mNormal = File::LoadTexture(data.mNormalURL);
		data.mTexture = File::LoadTexture(data.mTextureURL);
		auto material = new Material(std::move(data));
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
