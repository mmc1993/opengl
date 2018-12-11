#include "window.h"

#pragma comment(lib, "lib/glew32.lib")
#pragma comment(lib, "lib/glfw3.lib")
#pragma comment(lib, "OpenGL32.lib")

Window::Window()
    : _window(nullptr)
{
}

Window::~Window()
{
}

bool Window::Create(const std::string & title)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    _window = glfwCreateWindow(CW_DEFAULT, CW_DEFAULT, title.c_str(), nullptr, nullptr);
    if (nullptr == _window)
    {
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(_window);
    glfwSetWindowUserPointer(_window, this);

    //  初始化GLEW
    if (GLEW_OK != glewInit())
    {
        _window = nullptr;
        glfwTerminate();
        return false;
    }
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
    glClearColor(0, 0, 0, 1);

    //  绑定Input
    glfwSetKeyCallback(_window, Window::OnKey);
    glfwSetCursorPosCallback(_window, Window::OnCur);
    glfwSetWindowSizeCallback(_window, Window::OnSize);
    glfwSetMouseButtonCallback(_window, Window::OnBtn);
    glfwSetWindowCloseCallback(_window, Window::OnClose);

    //  初始化各项数据
    _renderInfo.renderTM = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(_window))
    {
        Update();
    }

    _window = nullptr;

    glfwTerminate();
}

EventDispatcher & Window::GefEventDispatcher()
{
    return _eventDispatcher;
}

Timer & Window::GetTimer()
{
    return _timer;
}

Object & Window::GetRoot()
{
    return _root;
}

void Window::SetFPS(size_t ms)
{
    _renderInfo.renderCD = std::chrono::milliseconds(1000 / ms);
}

void Window::Update()
{
    auto now = std::chrono::high_resolution_clock::now();
    if (now >= _renderInfo.renderTM)
    {
        glfwPollEvents();

        auto diffTM= now - _renderInfo.renderTM;
        diffTM += std::chrono::milliseconds(16);
        _renderInfo.renderTM += _renderInfo.renderCD;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        _root.Update(std::chrono::duration_cast
            <std::chrono::milliseconds>
            (diffTM).count() * 0.001f);

        glfwSwapBuffers(_window);
    }
}

void Window::OnBtn(GLFWwindow * window, int btn, int act, int stat)
{
    EventMouse::Param param;
    param.act = act;
    param.btn = btn;
    param.stat = stat;
    reinterpret_cast<Window *>(glfwGetWindowUserPointer(window))->GefEventDispatcher().Post(EventMouse::kBUTTON, param);
}

void Window::OnCur(GLFWwindow * window, double x, double y)
{
    EventMouse::Param param;
    param.x = static_cast<float>(x);
    param.y = static_cast<float>(y);
    reinterpret_cast<Window *>(glfwGetWindowUserPointer(window))->GefEventDispatcher().Post(EventMouse::kMOVEED, param);
}

void Window::OnKey(GLFWwindow * window, int key, int scan, int act, int stat)
{
    EventKey::Param param;
    param.key = key;
    param.act = act;
    param.stat = stat;
    reinterpret_cast<Window *>(glfwGetWindowUserPointer(window))->GefEventDispatcher().Post(EventKey::kKEY, param);
}

void Window::OnSize(GLFWwindow * window, int w, int h)
{
    glViewport(0, 0, w, h);
}

void Window::OnClose(GLFWwindow * window)
{
}

