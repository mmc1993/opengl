#include "window.h"
#include "../event/event.h"
#include "../timer/timer.h"
#include "../render/render.h"
#include "../object/object.h"
#include "../event/event_enum.h"

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
    ASSERT_LOG(nullptr != _window, "_window Error");

    OnSize(_window, w, h);
    glfwSetWindowPos(_window, static_cast<int>(x), static_cast<int>(y));
    glfwSetWindowSize(_window, static_cast<int>(w), static_cast<int>(h));
}

void Window::SetFPS(size_t fps)
{
	_renderInfo.renderFPS = fps;
    //  渲染一帧需要的时间
    _renderInfo.renderTimeLast = 0.0f;
    _renderInfo.renderTimeStep = 1.0f / fps;
}

size_t Window::GetX() const
{
    ASSERT_LOG(nullptr != _window, "_window Error");

    int x, y;
    glfwGetWindowPos(_window, &x, &y);
    return static_cast<size_t>(x);
}

size_t Window::GetY() const
{
    ASSERT_LOG(nullptr != _window, "_window Error");

    int x, y;
    glfwGetWindowPos(_window, &x, &y);
    return static_cast<size_t>(y);
}

size_t Window::GetW() const
{
    ASSERT_LOG(nullptr != _window, "_window Error");

    int w, h;
    glfwGetWindowSize(_window, &w, &h);
    return static_cast<size_t>(w);
}

size_t Window::GetH() const
{
    ASSERT_LOG(nullptr != _window, "_window Error");

    int w, h;
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
    while (!glfwWindowShouldClose(_window)) { Update(); }
    _window = nullptr;
}

void Window::Update()
{
    auto lasttime = time_tool::Now();
    auto difftime = -_renderInfo.renderTimeLast + lasttime;
    if ( difftime >= _renderInfo.renderTimeStep)
    {
        auto dt = difftime / _renderInfo.renderTimeStep;
        
        _renderInfo.renderTimeLast = lasttime;

		glfwPollEvents();
        
		Global::Ref().RefTimer().Update(lasttime);
        Global::Ref().RefObject().RootUpdate(dt);
        Global::Ref().RefRender().RenderOnce();
        glfwSwapBuffers(_window);

		std::cout <<
			SFormat("Error: {0} FPS: {1} DiffTie: {2} RenderCount: {3} mVertexCount: {4}", 
					glGetError(), _renderInfo.renderFPS / dt, dt,
					Global::Ref().RefRender().GetRenderState().mRenderCount,
					Global::Ref().RefRender().GetRenderState().mVertexCount)
			<< std::endl;
    }
}

void Window::OnBtn(GLFWwindow * window, int btn, int act, int stat)
{
    EventMouseParam param;
    param.act = act;
    param.btn = btn;
    param.stat = stat;
    Global::Ref().RefEvent().Post(EventTypeEnum::kWINDOW_MOUSE_BUTTON, param);
}

void Window::OnCur(GLFWwindow * window, double x, double y)
{
    EventMouseParam param;
    param.x = static_cast<float>(x);
    param.y = static_cast<float>(y);
    Global::Ref().RefEvent().Post(EventTypeEnum::kWINDOW_MOUSE_MOVEED, param);
}

void Window::OnKey(GLFWwindow * window, int key, int scan, int act, int stat)
{
    EventKeyParam param;
    param.key = key;
    param.act = act;
    param.stat = stat;
    Global::Ref().RefEvent().Post(EventTypeEnum::kWINDOW_KEYBOARD, param);
}

void Window::OnSize(GLFWwindow * window, int w, int h)
{
    glViewport(0, 0, w, h);
}

void Window::OnClose(GLFWwindow * window)
{
}

