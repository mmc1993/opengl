#pragma once

#include "../include.h"

class Window {
public:
    struct RenderInfo {
		size_t renderFPS;
		float renderTime;
		float renderCD;
    };

public:
	enum EventType {
		kMOUSE_BUTTON,
		kMOUSE_MOVEED,
		kKEYBOARD,
	};

	struct EventMouseParam {
		int btn;
		int act;
		float x;
		float y;
		int stat;
	};

	struct EventKeyParam {
		int key;
		int act;
		int stat;
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

private:
    void Update();

private:
    static void OnBtn(GLFWwindow * window, int btn, int act, int stat);
    static void OnCur(GLFWwindow * window, double x, double y);
    static void OnKey(GLFWwindow * window, int key, int scan, int act, int stat);
    static void OnSize(GLFWwindow * window, int w, int h);
    static void OnClose(GLFWwindow * window);

private:
    GLFWwindow * _window;

    RenderInfo _renderInfo;
};