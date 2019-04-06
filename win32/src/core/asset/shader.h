#pragma once

#include "asset.h"
#include "../render/render_type.h"

class Bitmap;
class Texture;
class BitmapCube;

class Shader : public Asset {
public:
	static void SetUniform(GLuint GLID, const std::string & key, int val);
	static void SetUniform(GLuint GLID, const std::string & key, float val);
	static void SetUniform(GLuint GLID, const std::string & key, double val);
	static void SetUniform(GLuint GLID, const std::string & key, const glm::vec3 & val);
	static void SetUniform(GLuint GLID, const std::string & key, const glm::vec4 & val);
	static void SetUniform(GLuint GLID, const std::string & key, const glm::mat3 & val);
	static void SetUniform(GLuint GLID, const std::string & key, const glm::mat4 & val);
	static void SetUniform(GLuint GLID, const std::string & key, const Bitmap * val, size_t pos);
	static void SetUniform(GLuint GLID, const std::string & key, const Texture & val, size_t pos);
	static void SetUniform(GLuint GLID, const std::string & key, const BitmapCube * val, size_t pos);

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
    GLuint AddPass(const char * vs, 
				   const char * fs, 
				   const char * gs);

    void CheckPass(GLuint GLID, const std::string & string);

private:
    std::vector<RenderPass> _passs;
};