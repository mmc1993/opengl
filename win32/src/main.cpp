#include "core/mmc.h"
#include "core/timer/timer.h"
#include "core/event/event.h"
#include "core/window/window.h"
#include "core/object/camera.h"
#include "core/render/render.h"
#include "core/component/sprite.h"
#include "core/asset/asset_core.h"
#include "core/tools/debug_tool.h"
#include "core/component/transform.h"
#include "core/asset/file.h"

class AppWindow : public Window {
public:
    void InitGame()
    {
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

		mmc::mEvent.Add(Window::EventType::kKEYBOARD, [](const std::any & event) {
			auto param = std::any_cast<Window::EventKeyParam>(event);
			if (param.act != 0)
			{
				return;
			}

			switch (param.key)
			{
			case 265:
				mmc::mRoot.GetTransform()->AddScale(0.5, 0.5, 0.5);
				//mmc::mRoot.GetTransform()->AddRotate(1, 0, 0, -0.1f);
				break;
			case 264:
				mmc::mRoot.GetTransform()->AddRotate(1, 0, 0, 0.1f);
				break;
			}
			std::cout
				<< "key: " << param.key << "\n"
				<< "act: " << param.act << std::endl;
		});
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