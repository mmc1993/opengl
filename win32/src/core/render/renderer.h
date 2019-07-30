#pragma once

#include "../include.h"
#include "render_type.h"

class Pipe;
class PipeState;

class Renderer {
public:
    Renderer();
    ~Renderer();

    //  返回矩阵栈
    MatrixStack &   GetMaterialStack();
    //  返回1帧顶点数
    uint            GetVertexCount();
    //  返回1帧渲染次
    uint            GetDrawCount();
    //  渲染一帧
    void            RenderOnce();
    //  添加管道
    void            AddPipe(Pipe * pipe);
    //  清除命令
    void            ClearCommands();
    //  Bind 系函数. 提交数据且影响状态.
    void Bind(const CameraCommand * command);
    bool Bind(const GLProgram     * program, uint pass);
    //  Post 系函数. 提交数据不影响状态.
    void Post(const Light *         light);
    void Post(const glm::mat4  *    model);
    void Post(const GLMaterial *    material);
    void Post(const CommandEnum     type,       const RenderCommand * command);
    void Post(const DrawTypeEnum    type,       const GLMesh *           mesh);

private:
    PipeState *         _state;
    std::vector<Pipe *> _pipes;
};
