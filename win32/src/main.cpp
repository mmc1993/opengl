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
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

        auto camera = new Camera();
        camera->Init(90, (float)GetW(), (float)GetH(), 1, 500);
        camera->LookAt(
            glm::vec3(0, 0, 0), 
            glm::vec3(0, 0, -1), 
            glm::vec3(0, 1, 0));
        mmc::mRender.AddCamera(0, camera);

		auto material = File::LoadMaterial("res/material/1.txt");
		auto sprite = new Sprite();
		sprite->SetMaterial(material);
		mmc::mRoot.AddComponent(sprite);
		mmc::mRoot.GetTransform()->Translate(0, 0, -200);

		mmc::mEvent.Add(Window::EventType::kKEYBOARD, [this, camera](const std::any & event) {
			auto param = std::any_cast<Window::EventKeyParam>(event);
			
			if (param.act != 2)
			{
				return;
			}

			const auto angle = 0.1f;

			switch (param.key)
			{
			case 265:
				{
					auto rotate = mmc::mRoot.GetTransform()->GetRotateFromRoot();
					rotate = glm::inverse(rotate);
					auto aixs = glm::vec4(1, 0, 0, 1);
					mmc::mRoot.GetTransform()->AddRotate(rotate * aixs, -angle);
				}
				break;
			case 264:
				{
					auto rotate = mmc::mRoot.GetTransform()->GetRotateFromRoot();
					rotate = glm::inverse(rotate);
					auto aixs = glm::vec4(1, 0, 0, 1);
					mmc::mRoot.GetTransform()->AddRotate(rotate * aixs, angle);
				}
				break;
			case 263:
				{
					auto rotate = mmc::mRoot.GetTransform()->GetRotateFromRoot();
					rotate = glm::inverse(rotate);
					auto aixs = glm::vec4(0, 1, 0, 1);
					mmc::mRoot.GetTransform()->AddRotate(rotate * aixs, -angle);
				}
				break;
			case 262:
				{
					auto rotate = mmc::mRoot.GetTransform()->GetRotateFromRoot();
					rotate = glm::inverse(rotate);
					auto aixs = glm::vec4(0, 1, 0, 1);
					mmc::mRoot.GetTransform()->AddRotate(rotate * aixs, angle);
				}
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
    app.Move(200, 100, 600, 600);
    app.InitGame();
    app.SetFPS(60);
    app.Loop();
    return 0;
}