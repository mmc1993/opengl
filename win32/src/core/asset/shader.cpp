#include "shader.h"
#include "../third/sformat.h"
#include "../tools/debug_tool.h"
#include "../asset/texture.h"
#include "../asset/bitmap_cube.h"

Shader::Shader(const std::string & vs, const std::string & fs, const std::string & gs)
	: _GLID(0)
{
	Init(vs.c_str(), fs.c_str(), !gs.empty()? gs.c_str(): nullptr);
}

void Shader::Init(const char * vs, const char * fs, const char * gs)
{
	_GLID = glCreateProgram();

	if (vs != nullptr)
	{
		GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vshader, 1, &vs, nullptr);
		glCompileShader(vshader);
		Check(vshader, "vertex error");
		glAttachShader(_GLID, vshader);
		glDeleteShader(vshader);
	}

	if (fs != nullptr)
	{
		GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fshader, 1, &fs, nullptr);
		glCompileShader(fshader);
		Check(fshader, "fragment error");
		glAttachShader(_GLID, fshader);
		glDeleteShader(fshader);
	}
	
	if (gs != nullptr)
	{
		GLuint gshader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(gshader, 1, &gs, nullptr);
		glCompileShader(gshader);
		Check(gshader, "geometry error");
		glAttachShader(_GLID, gshader);
		glDeleteShader(gshader);
	}

	GLint ret;
	glLinkProgram(_GLID);
	glGetProgramiv(_GLID, GL_LINK_STATUS, &ret);
	if (ret == 0) { glDeleteProgram(_GLID); _GLID = 0; }
}

void Shader::Check(GLuint shader, const std::string & str)
{
	GLint ret;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ret);
	if (ret == 0)
	{
		char err[256] = { 0 };
		glGetShaderInfoLog(shader, sizeof(err), nullptr, err);
		std::cout << SFormat("Shader Error Desc: {0}, Code: {1}, Text: {2}", str, ret, err) << std::endl;
	}
}

Shader::~Shader()
{
    glDeleteProgram(_GLID);
}

void Shader::SetUniform(size_t idx, int val)
{
    glUniform1i(static_cast<GLint>(idx), val);
}

void Shader::SetUniform(size_t idx, float val)
{
    glUniform1f(static_cast<GLint>(idx), val);
}

void Shader::SetUniform(size_t idx, const glm::vec3 & val)
{
    glUniform3f(static_cast<GLint>(idx), val.x, val.y, val.z);
}

void Shader::SetUniform(size_t idx, const glm::vec4 & val)
{
    glUniform4f(static_cast<GLint>(idx), val.x, val.y, val.z, val.w);
}

void Shader::SetUniform(size_t idx, const glm::mat3 & val)
{
	glUniformMatrix3fv(idx, 1, GL_FALSE, &val[0][0]);
}

void Shader::SetUniform(size_t idx, const glm::mat4 & val)
{
	glUniformMatrix4fv(idx, 1, GL_FALSE, &val[0][0]);
}

void Shader::SetUniform(size_t idx, const Bitmap * val, size_t pos)
{
	glActiveTexture(GL_TEXTURE0 + pos);
	glBindTexture(GL_TEXTURE_2D, val->GetGLID());
	glUniform1i(static_cast<GLint>(idx), pos);
}

void Shader::SetUniform(size_t idx, const Texture & val, size_t pos)
{
	glActiveTexture(GL_TEXTURE0 + pos);
	glBindTexture(GL_TEXTURE_2D, val.GetBitmap()->GetGLID());
    glUniform1i(static_cast<GLint>(idx), pos);
}

void Shader::SetUniform(size_t idx, const BitmapCube * val, size_t pos)
{
	glActiveTexture(GL_TEXTURE0 + pos);
	glBindTexture(GL_TEXTURE_CUBE_MAP, val->GetGLID());
	glUniform1i(static_cast<GLint>(idx), pos);
}

void Shader::SetUniform(const std::string & key, int val)
{
    SetUniform(glGetUniformLocation(_GLID, key.c_str()), val);
}

void Shader::SetUniform(const std::string & key, float val)
{
    SetUniform(glGetUniformLocation(_GLID, key.c_str()), val);
}

void Shader::SetUniform(const std::string & key, const glm::vec3 & val)
{
    SetUniform(glGetUniformLocation(_GLID, key.c_str()), val);
}

void Shader::SetUniform(const std::string & key, const glm::vec4 & val)
{
    SetUniform(glGetUniformLocation(_GLID, key.c_str()), val);
}

void Shader::SetUniform(const std::string & key, const glm::mat3 & val)
{
	SetUniform(glGetUniformLocation(_GLID, key.c_str()), val);
}

void Shader::SetUniform(const std::string & key, const glm::mat4 & val)
{
	SetUniform(glGetUniformLocation(_GLID, key.c_str()), val);
}

void Shader::SetUniform(const std::string & key, const Bitmap * val, size_t pos)
{
	SetUniform(glGetUniformLocation(_GLID, key.c_str()), val, pos);
}

void Shader::SetUniform(const std::string & key, const Texture & val, size_t pos)
{
    SetUniform(glGetUniformLocation(_GLID, key.c_str()), val, pos);
}

void Shader::SetUniform(const std::string & key, const BitmapCube * val, size_t pos)
{
	SetUniform(glGetUniformLocation(_GLID, key.c_str()), val, pos);
}