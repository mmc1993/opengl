#include "shader.h"
#include "../third/sformat.h"

Shader::Shader(const std::string & vs, const std::string & fs): Shader(vs.c_str(), fs.c_str())
{

}

Shader::Shader(const char * vs, const char * fs)
{
	auto vret = 0;
	auto fret = 0;
	auto pret = 0;
	char logtxt[512] = { 0 };
	auto vshader = glCreateShader(GL_VERTEX_SHADER);
	auto fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vshader, 1, &vs, nullptr);
	glShaderSource(fshader, 1, &fs, nullptr);
	glCompileShader(vshader);
	glCompileShader(fshader);

	glGetShaderiv(vshader, GL_COMPILE_STATUS, &vret);
	glGetShaderiv(fshader, GL_COMPILE_STATUS, &fret);
	if (vret != 0 && fret != 0)
	{
		_GLID = glCreateProgram();
		glAttachShader(_GLID, vshader);
		glAttachShader(_GLID, fshader);
		glLinkProgram(_GLID);
		glGetProgramiv(_GLID, GL_LINK_STATUS, &pret);
	}
	if (vret == 0)
	{
		glGetShaderInfoLog(vshader, sizeof(logtxt), nullptr, logtxt);
		std::cout << SFormat("Shader Init Vertex Shader Failed: {0}", logtxt);
	}
	if (fret == 0)
	{
		glGetShaderInfoLog(fshader, sizeof(logtxt), nullptr, logtxt);
		std::cout << SFormat("Shader Init Fragment Shader Failed: {0}", logtxt);
	}
	if (pret == 0)
	{
		glGetProgramInfoLog(_GLID, sizeof(logtxt), nullptr, logtxt);
		std::cout << SFormat("Shader Init Program Shader Failed: {0}", logtxt);
	}
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	if (vret == 0 || fret == 0 || pret == 0)
	{
		_GLID = 0;
	}
}

Shader::~Shader()
{
    glDeleteProgram(_GLID);
}

bool Shader::InitFromFile(const std::string & vs, const std::string & fs)
{
    std::ifstream vfile(vs);
    std::ifstream ffile(fs);
    if (!vfile || !ffile) { return false; }
    std::stringstream vss, fss;
    vss << vfile.rdbuf();
    fss << ffile.rdbuf();
    vfile.close();
    ffile.close();
    return Init(vss.str(), fss.str());
}

void Shader::Bind()
{
    assert(_GLID != 0);
    glUseProgram(_GLID);
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

void Shader::SetUniform(size_t idx, const Texture & val)
{
    glUniform1i(static_cast<GLint>(idx), val.GetBitmap()->GetGLID());
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

void Shader::SetUniform(const std::string & key, const Texture & val)
{
    SetUniform(glGetUniformLocation(_GLID, key.c_str()), val);
}

