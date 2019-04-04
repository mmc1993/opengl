#include "shader.h"
#include "../asset/texture.h"
#include "../tools/debug_tool.h"
#include "../asset/bitmap_cube.h"

Shader::~Shader()
{
    for (auto & pass : _passs)
    {
        glDeleteProgram(pass.GLID);
    }
}

bool Shader::AddPass(const Pass & pass, const std::string & vs, const std::string & fs, const std::string & gs)
{
    auto GLID = AddPass(vs.c_str(), fs.c_str(), !gs.empty() ? gs.c_str() : nullptr);
    if (GLID != 0)
    {
        _passs.push_back(pass);
        _passs.back().GLID = GLID;
    }
    return GLID;
}

GLuint Shader::AddPass(const char * vs, const char * fs, const char * gs)
{
	auto GLID = glCreateProgram();

	if (vs != nullptr)
	{
		GLuint vsh = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vsh, 1, &vs, nullptr);
		glCompileShader(vsh);
		CheckPass(vsh, "Vertex Error");
		glAttachShader(GLID, vsh);
		glDeleteShader(vsh);
	}

	if (fs != nullptr)
	{
		GLuint fsh = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fsh, 1, &fs, nullptr);
		glCompileShader(fsh);
		CheckPass(fsh, "Fragment Error");
		glAttachShader(GLID, fsh);
		glDeleteShader(fsh);
	}
	
	if (gs != nullptr)
	{
		GLuint gsh = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(gsh, 1, &gs, nullptr);
		glCompileShader(gsh);
		CheckPass(gsh, "Geometry Error");
		glAttachShader(GLID, gsh);
		glDeleteShader(gsh);
	}

	GLint ret;
	glLinkProgram(GLID);
	glGetProgramiv(GLID, GL_LINK_STATUS, &ret);
	if (ret == 0) { glDeleteProgram(GLID); GLID = 0; }
    return GLID;
}

void Shader::CheckPass(GLuint GLID, const std::string & str)
{
	GLint ret;
	glGetShaderiv(GLID, GL_COMPILE_STATUS, &ret);
	if (ret == 0)
	{
		char err[256] = { 0 };
		glGetShaderInfoLog(GLID, sizeof(err), nullptr, err);
        ASSERT_LOG(false, "Shader Error Desc: {0}, Code: {1}, Text: {2}", str, ret, err);
	}
}

void Shader::SetUniform(size_t idx, int val)
{
    glUniform1i(static_cast<GLint>(idx), val);
}

void Shader::SetUniform(size_t idx, float val)
{
    glUniform1f(static_cast<GLint>(idx), val);
}

void Shader::SetUniform(size_t idx, double val)
{
    glUniform1f(static_cast<GLint>(idx), static_cast<float>(val));
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
    SetUniform(idx, val.GetBitmap(), pos);
}

void Shader::SetUniform(size_t idx, const BitmapCube * val, size_t pos)
{
	glActiveTexture(GL_TEXTURE0 + pos);
	glBindTexture(GL_TEXTURE_CUBE_MAP, val->GetGLID());
	glUniform1i(static_cast<GLint>(idx), pos);
}

void Shader::SetUniform(size_t pass, const std::string & key, int val)
{
    SetUniform(glGetUniformLocation(_passs.at(pass).GLID, key.c_str()), val);
}

void Shader::SetUniform(size_t pass, const std::string & key, float val)
{
    SetUniform(glGetUniformLocation(_passs.at(pass).GLID, key.c_str()), val);
}

void Shader::SetUniform(size_t pass, const std::string & key, double val)
{
    SetUniform(glGetUniformLocation(_passs.at(pass).GLID, key.c_str()), val);
}

void Shader::SetUniform(size_t pass, const std::string & key, const glm::vec3 & val)
{
    SetUniform(glGetUniformLocation(_passs.at(pass).GLID, key.c_str()), val);
}

void Shader::SetUniform(size_t pass, const std::string & key, const glm::vec4 & val)
{
    SetUniform(glGetUniformLocation(_passs.at(pass).GLID, key.c_str()), val);
}

void Shader::SetUniform(size_t pass, const std::string & key, const glm::mat3 & val)
{
	SetUniform(glGetUniformLocation(_passs.at(pass).GLID, key.c_str()), val);
}

void Shader::SetUniform(size_t pass, const std::string & key, const glm::mat4 & val)
{
	SetUniform(glGetUniformLocation(_passs.at(pass).GLID, key.c_str()), val);
}

void Shader::SetUniform(size_t pass, const std::string & key, const Bitmap * val, size_t pos)
{
	SetUniform(glGetUniformLocation(_passs.at(pass).GLID, key.c_str()), val, pos);
}

void Shader::SetUniform(size_t pass, const std::string & key, const Texture & val, size_t pos)
{
    SetUniform(glGetUniformLocation(_passs.at(pass).GLID, key.c_str()), val, pos);
}

void Shader::SetUniform(size_t pass, const std::string & key, const BitmapCube * val, size_t pos)
{
	SetUniform(glGetUniformLocation(_passs.at(pass).GLID, key.c_str()), val, pos);
}