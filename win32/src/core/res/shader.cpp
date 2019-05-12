#include "shader.h"
#include "bitmap.h"
#include "bitmap_cube.h"
#include "../tools/debug_tool.h"

void Shader::SetUniform(uint GLID, const char * key, iint val)
{
    glUniform1i(glGetUniformLocation(GLID, key), val);
}

void Shader::SetUniform(uint GLID, const char * key, uint val)
{
    glUniform1i(glGetUniformLocation(GLID, key), val);
}

void Shader::SetUniform(uint GLID, const char * key, float val)
{
	glUniform1f(glGetUniformLocation(GLID, key), val);
}

void Shader::SetUniform(uint GLID, const char * key, double val)
{
	glUniform1f(glGetUniformLocation(GLID, key), static_cast<float>(val));
}

void Shader::SetUniform(uint GLID, const char * key, const glm::vec3 & val)
{
	glUniform3f(glGetUniformLocation(GLID, key), val.x, val.y, val.z);
}

void Shader::SetUniform(uint GLID, const char * key, const glm::vec4 & val)
{
	glUniform4f(glGetUniformLocation(GLID, key), val.x, val.y, val.z, val.w);
}

void Shader::SetUniform(uint GLID, const char * key, const glm::mat3 & val)
{
	glUniformMatrix3fv(glGetUniformLocation(GLID, key), 1, GL_FALSE, &val[0][0]);
}

void Shader::SetUniform(uint GLID, const char * key, const glm::mat4 & val)
{

	glUniformMatrix4fv(glGetUniformLocation(GLID, key), 1, GL_FALSE, &val[0][0]);
}

void Shader::SetTexture2D(uint GLID, const char * key, const uint val, iint pos)
{
    glActiveTexture(GL_TEXTURE0 +pos);
    glBindTexture(GL_TEXTURE_2D, val);
    glUniform1i(glGetUniformLocation(GLID, key), pos);
}

void Shader::SetTexture3D(uint GLID, const char * key, const uint val, iint pos)
{
    glActiveTexture(GL_TEXTURE0 + pos);
    glBindTexture(GL_TEXTURE_CUBE_MAP, val);
    glUniform1i(glGetUniformLocation(GLID, key), pos);
}

void Shader::SetUniform(uint GLID, const char * key, const Bitmap * val, iint pos)
{
    SetTexture2D(GLID, key, val->GetGLID(), pos);
}

void Shader::SetUniform(uint GLID, const char * key, const BitmapCube * val, iint pos)
{
    SetTexture3D(GLID, key, val->GetGLID(), pos);
}

void Shader::UnbindTex2D(iint pos)
{
    glActiveTexture(GL_TEXTURE0 + pos);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Shader::UnbindTex3D(iint pos)
{
    glActiveTexture(GL_TEXTURE0 + pos);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

Shader::~Shader()
{
    for (auto & pass : _passs)
    {
        glDeleteProgram(pass.GLID);
    }
}

void Shader::AddPass(
    const Pass & pass, 
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
    ASSERT_LOG(GLID != 0, "vs: {0}, fs: {1}, gs: {2}", vs, fs, gs);
}

uint Shader::AddPass(const char * vs, const char * fs, const char * gs)
{
	auto GLID = glCreateProgram();

	if (vs != nullptr)
	{
		uint vsh = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vsh, 1, &vs, nullptr);
		glCompileShader(vsh);
		CheckPass(vsh, "Vertex Error");
		glAttachShader(GLID, vsh);
		glDeleteShader(vsh);
	}

	if (fs != nullptr)
	{
		uint fsh = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fsh, 1, &fs, nullptr);
		glCompileShader(fsh);
		CheckPass(fsh, "Fragment Error");
		glAttachShader(GLID, fsh);
		glDeleteShader(fsh);
	}
	
	if (gs != nullptr)
	{
		uint gsh = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(gsh, 1, &gs, nullptr);
		glCompileShader(gsh);
		CheckPass(gsh, "Geometry Error");
		glAttachShader(GLID, gsh);
		glDeleteShader(gsh);
	}

	iint ret;
	glLinkProgram(GLID);
	glGetProgramiv(GLID, GL_LINK_STATUS, &ret);
	if (ret == 0) { glDeleteProgram(GLID); GLID = 0; }
    return GLID;
}

void Shader::CheckPass(uint GLID, const std::string & str)
{
	iint ret;
	glGetShaderiv(GLID, GL_COMPILE_STATUS, &ret);
	if (ret == 0)
	{
		char err[256] = { 0 };
		glGetShaderInfoLog(GLID, sizeof(err), nullptr, err);
        ASSERT_LOG(false, "Shader Error Desc: {0}, Code: {1}, Text: {2}", str, ret, err);
	}
}

