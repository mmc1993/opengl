#pragma once

#include "../include.h"

class Shader;

class Render {
public:
    enum RenderQueueFlag {
        //  背景
        kBACKGROUND,
        //  集合
        kGEOMETRY,
        //  透明
        kOPATCIY,
        //  顶层
        kOVERLAY,
        //  标记最大值
        MAX,
    };

    struct Command {
        Shader * mShader;
        std::function<void()> mCallFn;
    };

public:
    Render();

    ~Render();

    void PostCommand(Shader * shader, const std::function<void ()> & callfn);

    void DoRender();

private:
    std::vector<std::vector<Command>> _renderQueue;
};