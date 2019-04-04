#pragma once

#include "asset.h"
#include "../render/render_type.h"

class Bitmap;
class Texture;
class BitmapCube;

class Shader : public Asset {
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

    void SetUniform(size_t idx, int val);
    void SetUniform(size_t idx, float val);
    void SetUniform(size_t idx, double val);
    void SetUniform(size_t idx, const glm::vec3 & val);
    void SetUniform(size_t idx, const glm::vec4 & val);
    void SetUniform(size_t idx, const glm::mat3 & val);
    void SetUniform(size_t idx, const glm::mat4 & val);
    void SetUniform(size_t idx, const Bitmap * val, size_t pos);
    void SetUniform(size_t idx, const Texture & val, size_t pos);
    void SetUniform(size_t idx, const BitmapCube * val, size_t pos);

    void SetUniform(size_t pass, const std::string & key, int val);
    void SetUniform(size_t pass, const std::string & key, float val);
    void SetUniform(size_t pass, const std::string & key, double val);
    void SetUniform(size_t pass, const std::string & key, const glm::vec3 & val);
    void SetUniform(size_t pass, const std::string & key, const glm::vec4 & val);
    void SetUniform(size_t pass, const std::string & key, const glm::mat3 & val);
    void SetUniform(size_t pass, const std::string & key, const glm::mat4 & val);
    void SetUniform(size_t pass, const std::string & key, const Bitmap * val, size_t pos);
    void SetUniform(size_t pass, const std::string & key, const Texture & val, size_t pos);
    void SetUniform(size_t pass, const std::string & key, const BitmapCube * val, size_t pos);

private:
    GLuint AddPass(const char * vs, const char * fs, const char * gs);
    void CheckPass(GLuint GLID, const std::string & string);

private:
    std::vector<RenderPass> _passs;
};