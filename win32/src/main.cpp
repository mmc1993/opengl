#include "core/timer/timer.h"
#include "core/window/window.h"

int main()
{
    Window window;
    window.Create("xxx");
    window.Move(200, 100, 800, 600);
    window.Loop();
    return 0;
}