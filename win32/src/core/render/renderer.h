#pragma once

#include "../include.h"
#include "render_type.h"

class Pipe;
class PipeState;

class Renderer {
public:
    Renderer();
    ~Renderer();

    MatrixStack &   GetMatrixStack();
    uint            GetVertexCount();
    uint            GetRenderCount();
    void            AddPipe(Pipe * pipe);
    void            RenderOnce();
    void            ClearCommands();
    void            SortLightCommands();

    //  Bind 系函数. 提交数据且影响状态.
    void Bind(const CameraCommand * command);
    bool Bind(const GLProgram * program, uint pass);
    //  Post 系函数. 提交数据不影响状态.
    void Post(const glm::mat4   * model);
    void Post(const GLMaterial  * material);
    void Post(const LightCommand * command);
    void Post(const CommandEnum     type, const RenderCommand * command);
    void Post(const DrawTypeEnum    draw, const FragTypeEnum frag, const GLMesh * mesh);

    void Init();
private:
    PipeState *         _state;
    std::vector<Pipe *> _pipes;
};
