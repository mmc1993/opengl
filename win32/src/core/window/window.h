#pragma once

#include "../include.h"
#include "../third/glew.h"
#include "../third/glfw3.h"

class Window {
public:
    Window();
    ~Window();
    bool Create(const std::string & title, size_t w, size_t h);
    void Move(size_t x);
    void Move(size_t x, size_t y);
    void Move(size_t x, size_t y, size_t w);
    void Move(size_t x, size_t y, size_t w, size_t h);
    size_t GetX() const;
    size_t GetY() const;
    size_t GetW() const;
    size_t GetH() const;
    void Loop();

private:
    static void OnBtn(GLFWwindow * window, int btn, int act, int stat);
    static void OnCur(GLFWwindow * window, double x, double y);
    static void OnKey(GLFWwindow * window, int key, int scan, int act, int stat);
    static void OnSize(GLFWwindow * window, int w, int h);
    static void OnClose(GLFWwindow * window);

private:
    GLFWwindow * _window;
};