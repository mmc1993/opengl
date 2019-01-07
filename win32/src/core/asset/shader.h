#pragma once

#include "asset.h"
#include "../math/vec4.h"
#include "../asset/texture.h"
#include "../render/render.h"

class Shader: public Asset {
public:
    struct Info {
        bool mIsDTest;
        bool mIsDWrite;
        bool mIsZTest;
        bool mIsZWrite;
        Render::QueueType mQueueType;
    };

public:
    Shader(const std::string & vs, const std::string & fs);
	Shader(const char * vs, const char * fs);
    ~Shader();

    void Bind();
    void SetUniform(size_t idx, int val);
    void SetUniform(size_t idx, float val);
    void SetUniform(size_t idx, const glm::vec3 & val);
    void SetUniform(size_t idx, const glm::vec4 & val);
    void SetUniform(size_t idx, const Texture & val);

    void SetUniform(const std::string & key, int val);
    void SetUniform(const std::string & key, float val);
    void SetUniform(const std::string & key, const glm::vec3 & val);
    void SetUniform(const std::string & key, const glm::vec4 & val);
    void SetUniform(const std::string & key, const Texture & val);

    GLuint GetGLID() const { return _GLID; }
    Info & GetInfo() { return _info; }
private:
    GLuint _GLID;
    Info _info;
};