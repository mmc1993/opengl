#pragma once

#include "asset.h"
#include "../math/vec4.h"
#include "../asset/texture.h"

class Shader: public Asset {
public:
    Shader(const std::string & vs, const std::string & fs);
    ~Shader();

    void SetUniform(size_t idx, int val);
    void SetUniform(size_t idx, float val);
    void SetUniform(size_t idx, const glm::vec3 & val);
    void SetUniform(size_t idx, const glm::vec4 & val);
	void SetUniform(size_t idx, const glm::mat3 & val);
	void SetUniform(size_t idx, const glm::mat4 & val);
	void SetUniform(size_t idx, const Texture & val, size_t pos);

    void SetUniform(const std::string & key, int val);
    void SetUniform(const std::string & key, float val);
    void SetUniform(const std::string & key, const glm::vec3 & val);
	void SetUniform(const std::string & key, const glm::vec4 & val);
	void SetUniform(const std::string & key, const glm::mat3 & val);
	void SetUniform(const std::string & key, const glm::mat4 & val);
	void SetUniform(const std::string & key, const Texture & val, size_t pos);

    GLuint GetGLID() const 
	{ 
		assert(_GLID != 0);
		return _GLID; 
	}

private:
	bool Init(const char * vs, const char * fs);

private:
    GLuint _GLID;
};