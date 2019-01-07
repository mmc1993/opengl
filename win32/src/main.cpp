#include "core/mmc.h"
#include "core/timer/timer.h"
#include "core/window/window.h"
#include "core/object/camera.h"
#include "core/render/render.h"
#include "core/asset/asset_core.h"
#include "core/tools/debug_tool.h"
#include "core/asset/file.h"

class AppWindow : public Window {
public:
    void InitGame()
    {
        auto camera = new Camera();
        camera->Init(60, (float)GetW() / (float)GetH(), 1, 800);
        camera->LookAt(
            glm::vec3(0, 0, -1), 
            glm::vec3(0, 0, 0), 
            glm::vec3(0, 1, 0));
        mmc::mRender.AddCamera(camera, 0);

		auto material = File::LoadMaterial("res/material/1.txt");
    }
};

int main()
{
    AppWindow app;
    app.Create("xxx");
    app.Move(200, 100, 800, 600);
    app.InitGame();
    app.SetFPS(60);
    app.Loop();
    return 0;
}