#pragma once

#include "../include.h"

class Window {
public:
    struct RenderInfo {
		size_t renderFPS;
        float renderTimeStep;
        float renderTimeLast;
    };

public:
	struct EventMouseParam {
		int btn;
		int act;
		float x;
		float y;
        float dx;
        float dy;
		int stat;
	};

	struct EventKeyParam {
        float x;
        float y;
		int key;
		int act;
		int stat;
	};

    struct MouseInfo {
        iint x;
        iint y;
    };

public:
    Window();
    ~Window();
    bool Create(const std::string & title);
    void Move(size_t x);
    void Move(size_t x, size_t y);
    void Move(size_t x, size_t y, size_t w);
    void Move(size_t x, size_t y, size_t w, size_t h);
    void SetFPS(size_t fps);
    size_t GetX() const;
    size_t GetY() const;
    size_t GetW() const;
    size_t GetH() const;
    void Loop();

protected:
    virtual void OnGLError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message) const;

private:
    void Update();

private:
    //  鼠标点击
    static void OnMouseButton(GLFWwindow * window, int btn, int act, int stat);
    //  鼠标移动
    static void OnMouseMove(GLFWwindow * window, double x, double y);
    //  键盘按下/抬起
    static void OnKeybord(GLFWwindow * window, int key, int scan, int act, int stat);
    //  窗口尺寸变化
    static void OnSize(GLFWwindow * window, int w, int h);
    //  关闭窗口
    static void OnClose(GLFWwindow * window);
    //  调试消息回调
    static void GLAPIENTRY OnGLDebugProc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

private:
    MouseInfo _mouseInfo;
    GLFWwindow * _window;
    RenderInfo _renderInfo;
};