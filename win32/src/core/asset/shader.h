#pragma once

#include "asset.h"
#include "../render/render_type.h"

class Bitmap;
class BitmapCube;

class Shader : public Res {
public:
    static void SetUniform(uint GLID, const std::string & key, iint val);
    static void SetUniform(uint GLID, const std::string & key, uint val);
	static void SetUniform(uint GLID, const std::string & key, float val);
	static void SetUniform(uint GLID, const std::string & key, double val);
	static void SetUniform(uint GLID, const std::string & key, const glm::vec3 & val);
	static void SetUniform(uint GLID, const std::string & key, const glm::vec4 & val);
	static void SetUniform(uint GLID, const std::string & key, const glm::mat3 & val);
	static void SetUniform(uint GLID, const std::string & key, const glm::mat4 & val);
	static void SetUniform(uint GLID, const std::string & key, const Bitmap * val, iint pos);
	static void SetUniform(uint GLID, const std::string & key, const BitmapCube * val, iint pos);
    static void SetUniformTexArray2D(uint GLID, const std::string & key, const uint tex, iint pos);
    static void SetUniformTexArray3D(uint GLID, const std::string & key, const uint tex, iint pos);

public:
    ~Shader();

    bool IsEmpty() const
    {
        return _passs.empty();
    }

    const RenderPass & GetPass(size_t idx) const
    {
        return _passs.at(idx);
    }

    const std::vector<RenderPass> & GetPasss() const
    {
        return _passs;
    }

    bool AddPass(
        const RenderPass & pass,
        const std::string & vs,
        const std::string & fs,
        const std::string & gs);

private:
    uint AddPass(const char * vs, 
				   const char * fs, 
				   const char * gs);

    void CheckPass(uint GLID, const std::string & string);

private:
    std::vector<RenderPass> _passs;
};