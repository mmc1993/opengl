#include "window.h"
#include "../mmc.h"
#include "../event/event.h"
#include "../timer/timer.h"
#include "../asset/shader.h"
#include "../render/render.h"
#include "../object/object.h"

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
    OnSize(_window, w, h);
    glfwSetWindowPos(_window, static_cast<int>(x), static_cast<int>(y));
    glfwSetWindowSize(_window, static_cast<int>(w), static_cast<int>(h));
}

void Window::SetFPS(size_t ms)
{
    _renderInfo.renderCD = std::chrono::milliseconds(1000 / ms);
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
    while (!glfwWindowShouldClose(_window)) { Update(); }
    _window = nullptr;
    glfwTerminate();
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
        mmc::mRoot.Update(std::chrono::duration_cast
            <std::chrono::milliseconds>
            (diffTM).count() * 0.001f);
        mmc::mTimer.Update(now);
        mmc::mRender.DoRender();

        glfwSwapBuffers(_window);
    }
}

void Window::OnBtn(GLFWwindow * window, int btn, int act, int stat)
{
    EventMouse::Param param;
    param.act = act;
    param.btn = btn;
    param.stat = stat;
    mmc::mEvent.Post(EventMouse::kBUTTON, param);
}

void Window::OnCur(GLFWwindow * window, double x, double y)
{
    EventMouse::Param param;
    param.x = static_cast<float>(x);
    param.y = static_cast<float>(y);
    mmc::mEvent.Post(EventMouse::kMOVEED, param);
}

void Window::OnKey(GLFWwindow * window, int key, int scan, int act, int stat)
{
    EventKey::Param param;
    param.key = key;
    param.act = act;
    param.stat = stat;
    mmc::mEvent.Post(EventKey::kKEY, param);
}

void Window::OnSize(GLFWwindow * window, int w, int h)
{
    glViewport(0, 0, w, h);
}

void Window::OnClose(GLFWwindow * window)
{
}

