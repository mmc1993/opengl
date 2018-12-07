#include "window.h"

#pragma comment(lib, "lib/glew32.lib")
#pragma comment(lib, "lib/glfw3.lib")

Window::Window()
    : _window(nullptr)
{
}

Window::~Window()
{
}

bool Window::Create(const std::string & title, size_t w, size_t h)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    _window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
    if (nullptr == _window)
    {
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(_window);
    glfwSetWindowUserPointer(_window, this);
    return true;
}

void Window::Move(size_t x)
{
    Move(x, GetY(), GetW(), GetH());
}

void Window::Move(size_t x, size_t y)
{
    Move(x, y, GetW(), GetH());
}

void Window::Move(size_t x, size_t y, size_t w)
{
    Move(x, y, w, GetH());
}

void Window::Move(size_t x, size_t y, size_t w, size_t h)
{
    assert(nullptr != _window);
    glfwSetWindowPos(_window, x, y);
    glfwSetWindowSize(_window, w, h);
}

size_t Window::GetX() const
{
    int x, y;
    assert(nullptr != _window);
    glfwGetWindowPos(_window, &x, &y);
    return static_cast<size_t>(x);
}

size_t Window::GetY() const
{
    int x, y;
    assert(nullptr != _window);
    glfwGetWindowPos(_window, &x, &y);
    return static_cast<size_t>(y);
}

size_t Window::GetW() const
{
    int w, h;
    assert(nullptr != _window);
    glfwGetWindowSize(_window, &w, &h);
    return static_cast<size_t>(w);
}

size_t Window::GetH() const
{
    int w, h;
    assert(nullptr != _window);
    glfwGetWindowSize(_window, &w, &h);
    return static_cast<size_t>(h);
}

void Window::Loop()
{
    glfwSetKeyCallback(_window, Window::OnKey);
    glfwSetCursorPosCallback(_window, Window::OnCur);
    glfwSetWindowSizeCallback(_window, Window::OnSize);
    glfwSetMouseButtonCallback(_window, Window::OnBtn);
    glfwSetWindowCloseCallback(_window, Window::OnClose);
    while (!glfwWindowShouldClose(_window))
    {
        glfwPollEvents();
    }
    _window = nullptr;
    glfwTerminate();
}

void Window::OnBtn(GLFWwindow * window, int btn, int act, int stat)
{
    std::cout 
        << "onBtn "
        << btn << " " 
        << act << " "
        << stat << std::endl;
}

void Window::OnCur(GLFWwindow * window, double x, double y)
{
    std::cout
        << "OnCur"
        << x << " "
        << y << std::endl;
}

void Window::OnKey(GLFWwindow * window, int key, int scan, int act, int stat)
{
    std::cout
        << "OnKey"
        << key << " "
        << act << " "
        << stat << std::endl;
}

void Window::OnSize(GLFWwindow * window, int w, int h)
{
}

void Window::OnClose(GLFWwindow * window)
{
}

