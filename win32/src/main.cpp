#include "core/mmc.h"
#include "core/timer/timer.h"
#include "core/window/window.h"
#include "core/object/camera.h"
#include "core/render/render.h"
#include "core/component/sprite.h"
#include "core/asset/asset_core.h"
#include "core/tools/debug_tool.h"
#include "core/asset/file.h"

class AppWindow : public Window {
public:
    void InitGame()
    {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        auto camera = new Camera();
        camera->Init(90, (float)GetW() / (float)GetH(), 100, 800);
        camera->LookAt(
            glm::vec3(0, 0, -1), 
            glm::vec3(0, 0, 0), 
            glm::vec3(0, 1, 0));
        mmc::mRender.AddCamera(camera, 0);

		auto material = File::LoadMaterial("res/material/1.txt");
		auto sprite = new Sprite();
		sprite->SetMaterial(material);
		mmc::mRoot.AddComponent(sprite);
		mmc::mRoot.GetTransform()->Translate(0, 0, -200);
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