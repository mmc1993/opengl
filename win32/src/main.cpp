#include "core/timer/timer.h"
#include "core/window/window.h"
#include "core/component/transform.h"

int main()
{
    Transform transform;
    transform.Rotate(1, 0, 0, 3.14f);
    auto rotate = transform.GetRotate();
    Window window;
    window.Create("xxx");
    window.Move(200, 100, 800, 600);
    window.SetFPS(60);
    window.Loop();
    return 0;
}