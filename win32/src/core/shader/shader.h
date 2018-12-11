#pragma once

#include "../include.h"
#include "../math/vec4.h"

class Shader {
public:
    struct Info {
        bool mIsDTest;
        bool mIsDWrite;
        bool mIsZTest;
        bool mIsZWrite;
    };

public:
    Shader();
    ~Shader();

    bool InitFromFile(const std::string & vs, const std::string & fs);
    bool Init(const std::string & vs, const std::string & fs);
    bool Init(const char * vs, const char * fs);
    void Bind();

    void SetUniform(size_t idx, int val);
    void SetUniform(size_t idx, float val);
    void SetUniform(size_t idx, const Vec4 & val);

    void SetUniform(const std::string & key, int val);
    void SetUniform(const std::string & key, float val);
    void SetUniform(const std::string & key, const Vec4 & val);

    GLuint GetID() const { return _ID; }

    Info & GetInfo() { return _info; }

private:
    GLuint _ID;
    Info _info;
};