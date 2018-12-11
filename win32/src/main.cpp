#include "core/timer/timer.h"
#include "core/window/window.h"
#include "core/component/transform.h"

class AppWindow : public Window {

};

int main()
{
    AppWindow app;
    app.Create("xxx");
    app.Move(200, 100, 800, 600);
    app.SetFPS(60);
    app.Loop();
    return 0;
}