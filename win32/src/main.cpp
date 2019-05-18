
#include "core/timer/timer.h"
#include "core/event/event.h"
#include "core/window/window.h"
#include "core/component/camera.h"
#include "core/render/render.h"
#include "core/res/shader.h"
#include "core/res/material.h"
#include "core/component/sprite.h"
#include "core/component/sprite_screen.h"
#include "core/component/light.h"
#include "core/component/transform.h"
#include "core/component/skybox.h"
#include "core/res/model.h"
#include "core/res/file.h"
#include "core/res/res_manager.h"
#include "core/cfg/cfg_cache.h"
#include "core/event/event_enum.h"
#include <filesystem>

class AppWindow : public Window {
public:
	const float s_MAX_SPEED = 0.1f;
	
	enum DirectEnum {
		kNONE,
		kFRONT = 1,
		kBACK = 2,
		kLEFT = 4,
		kRIGHT = 8,
		kUP = 16,
		kDOWN = 32,
	};

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
		auto camera = new Camera();
		camera->InitPerspective(60, (float)GetW(), (float)GetH(), 1.0f, 30000);
		camera->SetViewport({ 0, 0, GetW(), GetH() });
		camera->LookAt(
			glm::vec3(0, 10, 10),
			glm::vec3(0, 0, 0),
			glm::vec3(0, 1, 0));
		Global::Ref().RefRender().AddCamera(camera, Render::CameraInfo::kFLAG0, 0);
	}

	void InitAssets()
	{
        File::LoadShader(BUILTIN_SHADER_LIGHT);
	}

	void InitObject()
	{
        auto object = new Object();
        object->SetParent(&Global::Ref().RefObject());
        CreateObject(File::LoadModel("res/demo/scene.obj"), object,
                     "res/demo/shader/scene.shader", glm::vec3(0, 0, 0));
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
			{ glm::vec3(0, 10, 10), glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(0.5f, 0.5f, 0.5f), glm::normalize(glm::vec3(0, -1, -1)) },
		};

		//	坐标，环境光，漫反射，镜面反射，衰减k0, k1, k2
		const std::vector<std::array<glm::vec3, 5>> points = {
            { glm::vec3(-1.5f, 8, 3), glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.4f, 0.4f, 0.4f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0001f, 0.01f) },
		};

		//	坐标，环境，漫反射，镜面反射，方向，衰减k0, k1, k2，内切角，外切角
		const std::vector<std::array<glm::vec3, 7>> spots = {
			{ glm::vec3(-1.5f, 10, -3), glm::vec3(0.3f, 0.3f, 0.3f), glm::vec3(0.4f, 0.4f, 0.4f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0, -1, 0), glm::vec3(1.0f, 0.0001f, 0.01f), glm::vec3(0.9f, 0.8f, 0.0f) },
		};

		for (auto & data : directs)
		{
			auto light = new LightDirect();
			light->mAmbient = data[1];
			light->mDiffuse = data[2];
			light->mSpecular = data[3];
			light->mNormal = data[4];
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
		auto & param = std::any_cast<const Window::EventKeyParam &>(any);
		_direct = param.act == 1 && param.key == 'W' ? _direct | kFRONT :
				  param.act == 0 && param.key == 'W' ? _direct ^ kFRONT : _direct;
		
		_direct = param.act == 1 && param.key == 'S' ? _direct | kBACK:
				  param.act == 0 && param.key == 'S' ? _direct ^ kBACK : _direct;

		_direct = param.act == 1 && param.key == 'A' ? _direct | kLEFT :
				  param.act == 0 && param.key == 'A' ? _direct ^ kLEFT : _direct;

		_direct = param.act == 1 && param.key == 'D' ? _direct | kRIGHT :
				  param.act == 0 && param.key == 'D' ? _direct ^ kRIGHT : _direct;

		_direct = param.act == 1 && param.key == 'Q' ? _direct | kUP :
				  param.act == 0 && param.key == 'Q' ? _direct ^ kUP : _direct;

		_direct = param.act == 1 && param.key == 'E' ? _direct | kDOWN :
				  param.act == 0 && param.key == 'E' ? _direct ^ kDOWN : _direct;
	}

	void OnMouseButton(const std::any & any)
	{
	}

	void OnMouseMoveed(const std::any & any)
	{
		auto  param = std::any_cast<Window::EventMouseParam>(any);
		auto l = glm::vec2(GetW() * 0.5f, GetH() * 0.5f);
		auto v = glm::vec2(param.x - l.x, l.y - param.y);
		if (glm::length(v) < 100)
		{
			_axis.x = _axis.y = _axis.z = _speed = 0.0f;
		}
		else
		{
			auto camera = Global::Ref().RefRender().GetCamera(0);
			auto cos = std::acos(glm::dot(glm::vec2(1, 0), glm::normalize(v)));
			cos = v.y < 0 ? cos : -cos;

			auto right = glm::cross(camera->GetEye(), camera->GetUp());
			auto step = glm::angleAxis(cos, camera->GetEye()) * right;
			auto look = glm::normalize(camera->GetEye() + step);
			_axis = glm::normalize(glm::cross(camera->GetEye(), look));

			auto s = glm::length(v) / glm::length(l);
			_speed = s * AppWindow::s_MAX_SPEED;
		}
	}
	
	void OnTimerUpdate()
	{
		auto camera = Global::Ref().RefRender().GetCamera(0);
		if (_axis.x != 0 || _axis.y != 0 || _axis.z != 0)
		{
			camera->SetEye(glm::quat(glm::angleAxis(_speed, _axis)) * camera->GetEye());
		}
		if (_direct != 0)
		{
			auto pos = camera->GetPos();
			if ((_direct & kUP) != 0) { pos.y += 1 * 0.1f; }
			if ((_direct & kDOWN) != 0) { pos.y -= 1 * 0.1f; }
			if ((_direct & kFRONT) != 0) { pos += camera->GetEye() * 0.1f; }
			if ((_direct & kBACK) != 0) { pos -= camera->GetEye() * 0.1f; }
			if ((_direct & kLEFT) != 0) { pos += glm::cross(camera->GetUp(), camera->GetEye()) * 0.1f; }
			if ((_direct & kRIGHT) != 0) { pos -= glm::cross(camera->GetUp(), camera->GetEye()) * 0.1f; }
			camera->SetPos(pos);
		}

        _lightPoints.at(0)->GetOwner()->GetTransform()->Translate(-1.5f,
                                                        8 + std::cos(_pointCos) * 3,
                                                        3 + std::sin(_pointCos) * 3);
        _pointCos += 0.1f;

		_lightSpots.at(0)->GetOwner()->GetTransform()->Translate(4 + std::cos(_spotCos) * 3,
																 8,
																 0 + std::sin(_spotCos) * 5);
		_spotCos += 0.1f;

		Global::Ref().RefTimer().Add(0.016f, std::bind(&AppWindow::OnTimerUpdate, this));
	}

    void CreateObject(Model * model, Object * parent, const std::string & shaderURL, const glm::vec3 & pos)
    {
        auto sprite = new Sprite();
        sprite->BindShader(shaderURL);
        for (auto i = 0; i != model->mMeshs.size(); ++i)
        {
            sprite->AddMesh(model->mMeshs.at(i), model->mMates.at(i));
        }

        auto object = new Object();
        object->SetParent(parent);
        object->AddComponent(sprite);
        object->GetTransform()->Translate(pos);

        for (auto i = 0; i != model->mChilds.size(); ++i)
        {
            CreateObject(model->mChilds.at(i), object, shaderURL, pos);
        }
    };
	
private:
	std::vector<LightDirect *> _lightDirects;
	std::vector<LightPoint *> _lightPoints;
	std::vector<LightSpot *> _lightSpots;
	glm::vec3 _axis;
	float _speed;
	int _direct;

	float _spotCos;
	float _pointCos;
};

int main()
{
    Global::Ref().Start();

    auto renderFPS = Global::Ref().RefCfgCache().At("init")->At("render", "fps")->ToInt();

    auto windowX = Global::Ref().RefCfgCache().At("init")->At("window", "x")->ToInt();
    auto windowY = Global::Ref().RefCfgCache().At("init")->At("window", "y")->ToInt();
    auto windowW = Global::Ref().RefCfgCache().At("init")->At("window", "w")->ToInt();
    auto windowH = Global::Ref().RefCfgCache().At("init")->At("window", "h")->ToInt();
    auto windowTitle = Global::Ref().RefCfgCache().At("init")->At("window", "title")->ToString();

    AppWindow app;
    app.Create(windowTitle);
    app.Move(windowX, windowY, 
             windowW, windowH);
    app.SetFPS(renderFPS);
    app.InitGame();
    app.Loop();

    Global::Ref().Clean();

    return 0;
}