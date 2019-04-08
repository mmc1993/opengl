#include "shader.h"
#include "../asset/texture.h"
#include "../tools/debug_tool.h"
#include "../asset/bitmap_cube.h"

void Shader::SetUniform(GLuint GLID, const std::string & key, int val)
{
	glUniform1i(glGetUniformLocation(GLID, key.c_str()), val);
}

void Shader::SetUniform(GLuint GLID, const std::string & key, float val)
{

	glUniform1f(glGetUniformLocation(GLID, key.c_str()), val);
}

void Shader::SetUniform(GLuint GLID, const std::string & key, double val)
{
	glUniform1f(glGetUniformLocation(GLID, key.c_str()), static_cast<float>(val));
}

void Shader::SetUniform(GLuint GLID, const std::string & key, const glm::vec3 & val)
{
	glUniform3f(glGetUniformLocation(GLID, key.c_str()), val.x, val.y, val.z);
}

void Shader::SetUniform(GLuint GLID, const std::string & key, const glm::vec4 & val)
{
	glUniform4f(glGetUniformLocation(GLID, key.c_str()), val.x, val.y, val.z, val.w);
}

void Shader::SetUniform(GLuint GLID, const std::string & key, const glm::mat3 & val)
{
	glUniformMatrix3fv(glGetUniformLocation(GLID, key.c_str()), 1, GL_FALSE, &val[0][0]);
}

void Shader::SetUniform(GLuint GLID, const std::string & key, const glm::mat4 & val)
{

	glUniformMatrix4fv(glGetUniformLocation(GLID, key.c_str()), 1, GL_FALSE, &val[0][0]);
}

void Shader::SetUniform(GLuint GLID, const std::string & key, const Bitmap * val, size_t pos)
{
	glActiveTexture(GL_TEXTURE0 + pos);
	glBindTexture(GL_TEXTURE_2D, val->GetGLID());
	glUniform1i(glGetUniformLocation(GLID, key.c_str()), pos);
}

void Shader::SetUniform(GLuint GLID, const std::string & key, const Texture & val, size_t pos)
{
	glActiveTexture(GL_TEXTURE0 + pos);
	glBindTexture(GL_TEXTURE_2D, val.GetBitmap()->GetGLID());
	glUniform1i(glGetUniformLocation(GLID, key.c_str()), pos);
}

void Shader::SetUniform(GLuint GLID, const std::string & key, const BitmapCube * val, size_t pos)
{
	glActiveTexture(GL_TEXTURE0 + pos);
	glBindTexture(GL_TEXTURE_CUBE_MAP, GLID);
	glUniform1i(glGetUniformLocation(GLID, key.c_str()), pos);
}

Shader::~Shader()
{
    for (auto & pass : _passs)
    {
        glDeleteProgram(pass.GLID);
    }
}

bool Shader::AddPass(
    const RenderPass & pass, 
    const std::string & vs, 
    const std::string & fs, 
    const std::string & gs)
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

