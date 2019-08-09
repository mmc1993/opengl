
#include "core/timer/timer.h"
#include "core/event/event.h"
#include "core/window/window.h"
#include "core/component/camera.h"
#include "core/render/renderer.h"
#include "core/component/sprite.h"
#include "core/component/light.h"
#include "core/component/transform.h"
#include "core/component/roaming.h"
#include "core/raw/raw_manager.h"
#include "core/cfg/cfg_manager.h"
#include "core/event/event_enum.h"
#include "core/render/pipe_shadow.h"
#include "core/render/pipe_ssao.h"
#include "core/render/pipe_forward.h"
#include "core/render/pipe_deferred.h"
#include "core/render/pipe_gbuffer.h"
#include <filesystem>

class AppWindow : public Window {
public:
    void InitGame()
    {
        InitAssets();
		InitCamera();
		InitEvents();
		InitLights();
		InitObject();
	}
private:
	void InitCamera()
	{
        auto object = new Object();

		auto camera = new Camera();
		camera->InitPerspective(60, (float)GetW(), (float)GetH(), 1.0f, 100.0f);
		camera->SetViewport({ 0, 0, GetW(), GetH() });
		//camera->LookAt(
		//	glm::vec3(10.2153053, 18.6474419, 8.25051212),
		//	glm::vec3(9.65997696, 18.1241894, 7.60412455),
		//	glm::vec3(0, 1, 0));
        camera->LookAt(
            glm::vec3(0, 0, 5),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0));
        camera->SetMask(Camera::kMASK0);
        camera->SetOrder(0);
        object->AddComponent(camera);

        auto roaming = new Roaming();
        object->AddComponent(roaming);

        Global::Ref().RefObject().AddChild(object);
	}

	void InitAssets()
	{
	}

	void InitObject()
	{
        //auto sprite0 = new Sprite();
        //sprite0->BindMaterial(Global::Ref().RefRawManager().LoadRes<GLMaterial>("res/lambert/material/scene.mtl"));
        //auto object0 = new Object();
        //object0->AddComponent(sprite0);
        //object0->GetTransform()->Translate(0, 0, 0);
        //object0->SetParent(&Global::Ref().RefObject());

        auto sprite1 = new Sprite();
        sprite1->BindMaterial(Global::Ref().RefRawManager().LoadRes<GLMaterial>("res/lambert/material/ball.mtl"));
        auto object1 = new Object();
        object1->AddComponent(sprite1);
        object1->GetTransform()->Translate(0, 0, 0);
        object1->SetParent(&Global::Ref().RefObject());
	}

	void InitEvents()
	{
		Global::Ref().RefTimer().Add(0.016f, std::bind(&AppWindow::OnTimerUpdate, this));
		Global::Ref().RefEvent().Add(EventTypeEnum::kWINDOW_KEYBOARD, std::bind(&AppWindow::OnKeyEvent, this, std::placeholders::_1));
		Global::Ref().RefEvent().Add(EventTypeEnum::kWINDOW_MOUSE_BUTTON, std::bind(&AppWindow::OnMouseButton, this, std::placeholders::_1));
		Global::Ref().RefEvent().Add(EventTypeEnum::kWINDOW_MOUSE_MOVEED, std::bind(&AppWindow::OnMouseMoveed, this, std::placeholders::_1));
	}

	void InitLights()
	{
		//	坐标，环境光，漫反射，镜面反射，方向
		const std::vector<std::array<glm::vec3, 5>> directs = {
			{ glm::vec3(0, 10, 10), glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.1f, 0.1f, 0.1f), glm::normalize(glm::vec3(0, -1, -1)) },
		};

		//	坐标，环境光，漫反射，镜面反射，衰减k0, k1, k2
		const std::vector<std::array<glm::vec3, 5>> points = {
            //{ glm::vec3(8, 8, 3), glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.4f, 0.4f, 0.4f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0001f, 0.01f) },
		};

		//	坐标，环境，漫反射，镜面反射，方向，衰减k0, k1, k2，内切角，外切角
		const std::vector<std::array<glm::vec3, 7>> spots = {
			//{ glm::vec3(2.0f, 10, -3), glm::vec3(0.3f, 0.3f, 0.3f), glm::vec3(0.4f, 0.4f, 0.4f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0, -1, 0), glm::vec3(1.0f, 0.0001f, 0.01f), glm::vec3(0.9f, 0.8f, 0.0f) },
		};

		for (auto & data : directs)
		{
			auto light = new LightDirect();
			light->mAmbient = data[1];
			light->mDiffuse = data[2];
			light->mSpecular = data[3];
			light->mNormal = data[4];
            light->BindProgram(Global::Ref().RefRawManager().LoadRes<GLProgram>("res/lambert/program/deferred_light_volume.program"));
            light->OpenShadow({ -50, 50 }, { -50, 50 }, { -10, 1000 });
			auto object = new Object();
			object->AddComponent(light);
			object->GetTransform()->Translate(data[0]);
			object->SetParent(&Global::Ref().RefObject());
			_lightDirects.push_back(light);
		}

		for (auto & data : points)
		{
			auto light = new LightPoint();
			light->mAmbient = data[1];
			light->mDiffuse = data[2];
			light->mSpecular = data[3];
			light->mK0 = data[4].x;
			light->mK1 = data[4].y;
			light->mK2 = data[4].z;
            light->BindProgram(Global::Ref().RefRawManager().LoadRes<GLProgram>("res/lambert/program/deferred_light_volume.program"));
            light->OpenShadow(1, 100);
			auto object = new Object();
			object->AddComponent(light);
			object->GetTransform()->Translate(data[0]);
			object->SetParent(&Global::Ref().RefObject());
			_lightPoints.push_back(light);
		}

		for (auto & data : spots)
		{
			auto light = new LightSpot();
			light->mAmbient = data[1];
			light->mDiffuse = data[2];
			light->mSpecular = data[3];
			light->mNormal = data[4];
			light->mK0 = data[5].x;
			light->mK1 = data[5].y;
			light->mK2 = data[5].z;
			light->mInCone = data[6].x;
			light->mOutCone = data[6].y;
            light->BindProgram(Global::Ref().RefRawManager().LoadRes<GLProgram>("res/lambert/program/deferred_light_volume.program"));
            light->OpenShadow(1, 100);
			auto object = new Object();
			object->AddComponent(light);
			object->GetTransform()->Translate(data[0]);
			object->SetParent(&Global::Ref().RefObject());
			_lightSpots.push_back(light);
		}
	}

	void OnKeyEvent(const std::any & any)
	{
	}

	void OnMouseButton(const std::any & any)
	{
	}

	void OnMouseMoveed(const std::any & any)
	{
	}
	
	void OnTimerUpdate()
	{
		Global::Ref().RefTimer().Add(0.016f, std::bind(&AppWindow::OnTimerUpdate, this));
	}

private:
    //  Lights
	std::vector<LightDirect *> _lightDirects;
	std::vector<LightPoint *> _lightPoints;
	std::vector<LightSpot *> _lightSpots;
};

int main()
{
    AppWindow app;

    Global::Ref().Start();

    //  绑定App
    Global::Ref().BindWindow(&app);

    //  初始化配置文件
    Global::Ref().RefCfgManager().Init("res/config");

    //  创建窗口
    auto renderFPS = Global::Ref().RefCfgManager().At("init", "render", "fps")->ToInt();
    auto windowX = Global::Ref().RefCfgManager().At("init", "window", "x")->ToInt();
    auto windowY = Global::Ref().RefCfgManager().At("init", "window", "y")->ToInt();
    auto windowW = Global::Ref().RefCfgManager().At("init", "window", "w")->ToInt();
    auto windowH = Global::Ref().RefCfgManager().At("init", "window", "h")->ToInt();
    auto windowTitle = Global::Ref().RefCfgManager().At("init", "window", "title")->ToString();
    app.Create(windowTitle, windowX, windowY, windowW, windowH);
    app.SetFPS(renderFPS);

    //  初始化OpenGL
    auto opengl = Global::Ref().RefCfgManager().At("init", "open_gl");
    for (auto pair : opengl)
    {
        if (pair.mKey == "enable_primitive_restar")
        {
            glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
            glPrimitiveRestartIndex(pair.mValue->ToInt());
        }
    }

    //  初始化资源
    Global::Ref().RefRawManager().BegImport(true);
    Global::Ref().RefRawManager().Import(BUILTIN_MESH_DEFERRED_LIGHT_VOLUME_DIRECT);
    Global::Ref().RefRawManager().Import(BUILTIN_MESH_DEFERRED_LIGHT_VOLUME_POINT);
    Global::Ref().RefRawManager().Import(BUILTIN_MESH_DEFERRED_LIGHT_VOLUME_SPOT);
    Global::Ref().RefRawManager().Import(BUILTIN_MESH_SCREEN_QUAD);
    Global::Ref().RefRawManager().Import(BUILTIN_PROGRAM_SSAO);
    Global::Ref().RefRawManager().Import("res/lambert/program/deferred_light_volume.program");
    Global::Ref().RefRawManager().Import("res/lambert/program/deferred_gbuffer.program");
    Global::Ref().RefRawManager().Import("res/lambert/program/billboard.program");
    Global::Ref().RefRawManager().Import("res/lambert/program/forward.program");
    Global::Ref().RefRawManager().Import("res/lambert/ball.obj");
    Global::Ref().RefRawManager().Import("res/lambert/scene.obj");
    Global::Ref().RefRawManager().Import("res/lambert/wall.obj");
    Global::Ref().RefRawManager().Import("res/lambert/material/ball.mtl");
    Global::Ref().RefRawManager().Import("res/lambert/material/scene.mtl");
    Global::Ref().RefRawManager().Import("res/lambert/texture/ball_specular.png");
    Global::Ref().RefRawManager().Import("res/lambert/texture/ball_diffuse.png");
    Global::Ref().RefRawManager().Import("res/lambert/texture/ball_normal.png");
    Global::Ref().RefRawManager().EndImport();
    Global::Ref().RefRawManager().Init();

    //  初始化渲染器
    Global::Ref().RefRenderer().Init();
    Global::Ref().RefRenderer().AddPipe(    new PipeShadow()    );
    Global::Ref().RefRenderer().AddPipe(    new PipeGBuffer()   );
    Global::Ref().RefRenderer().AddPipe(    new PipeSSAO()      );
    Global::Ref().RefRenderer().AddPipe(    new PipeDeferred()  );
    Global::Ref().RefRenderer().AddPipe(    new PipeForward()   );

    //  自定义初始化
    app.InitGame();

    //  进入窗口循环
    app.Loop();

    //  结束
    Global::Ref().Clean();

    return 0;
}