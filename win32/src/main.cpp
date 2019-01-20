#include "core/mmc.h"
#include "core/timer/timer.h"
#include "core/event/event.h"
#include "core/window/window.h"
#include "core/object/camera.h"
#include "core/render/render.h"
#include "core/asset/shader.h"
#include "core/asset/material.h"
#include "core/component/sprite.h"
#include "core/asset/asset_core.h"
#include "core/tools/debug_tool.h"
#include "core/component/light.h"
#include "core/component/transform.h"
#include "core/asset/file.h"

class AppWindow : public Window {
public:
	const float s_MAX_SPEED = 0.01f;
	
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
		InitCamera();
		InitAssets();
		InitEvents();
		InitLights();
		InitBoxs();
    }
private:
	void InitCamera()
	{
		auto camera = new Camera();
		camera->Init(60, (float)GetW(), (float)GetH(), 1, 500);
		camera->LookAt(
			glm::vec3(0, 0, 3),
			glm::vec3(0, 0, -1),
			glm::vec3(0, 1, 0));
		mmc::mRender.AddCamera(0, camera);
	}

	void InitAssets()
	{
		File::LoadTexture("res/bitmap/awesomeface.png", GL_RGBA);
		File::LoadTexture("res/bitmap/container.png", GL_RGBA);
		File::LoadMaterial("res/material/1.txt");
		File::LoadShader("res/shader/1.shader");
		File::LoadShader("res/shader/light.shader");
		File::LoadMesh("res/model/1/model.obj");
	}

	void InitBoxs()
	{
		glm::vec3 points[] = {
			glm::vec3(0.0f,  0.0f,  0.0f),
			glm::vec3(2.0f,  5.0f, -15.0f),
			glm::vec3(-1.5f, -2.2f, -2.5f),
			glm::vec3(-3.8f, -2.0f, -12.3f),
			glm::vec3(2.4f, -0.4f, -3.5f),
			glm::vec3(-1.7f,  3.0f, -7.5f),
			glm::vec3(1.3f, -2.0f, -2.5f),
			glm::vec3(1.5f,  2.0f, -2.5f),
			glm::vec3(1.5f,  0.2f, -1.5f),
			glm::vec3(-1.3f,  1.0f, -1.5f)
		};

		for (auto & point : points)
		{
			auto sprite = new Sprite();
			sprite->SetMaterial(mmc::mAssetCore.Get<Material>("res/material/1.txt"));
			sprite->SetShader(mmc::mAssetCore.Get<Shader>("res/shader/1.shader"));
			sprite->SetMesh(mmc::mAssetCore.Get<Mesh>("res/model/1/model.obj"));

			auto child = new Object();
			child->GetTransform()->Translate(point);
			child->AddComponent(sprite);
			mmc::mRoot.AddChild(child);
		}
	}

	void InitEvents()
	{
		mmc::mTimer.Add(16, std::bind(&AppWindow::OnTimerUpdate, this));
		mmc::mEvent.Add(Window::EventType::kKEYBOARD, std::bind(&AppWindow::OnKeyEvent, this, std::placeholders::_1));
		mmc::mEvent.Add(Window::EventType::kMOUSE_BUTTON, std::bind(&AppWindow::OnMouseButton, this, std::placeholders::_1));
		mmc::mEvent.Add(Window::EventType::kMOUSE_MOVEED, std::bind(&AppWindow::OnMouseMoveed, this, std::placeholders::_1));
	}

	void InitLights()
	{
		auto light = new Light();
		light->mMesh = mmc::mAssetCore.Get<Mesh>("res/model/1/model.obj");
		light->mShader = mmc::mAssetCore.Get<Shader>("res/shader/light.shader");
		light->mAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
		light->mDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		light->mSpecular = glm::vec3(1.0f, 1.0f, 1.0f);

		auto lightObject = new Object();
		lightObject->AddComponent(light);
		lightObject->GetTransform()->Scale(0.2f);
		lightObject->GetTransform()->Translate(1.0f, 1.0f, 2.0f);
		lightObject->SetParent(&mmc::mRoot);
	}

	void OnKeyEvent(const std::any & any)
	{
		auto & param = std::any_cast<Window::EventKeyParam>(any);
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
		auto l = glm::vec3(GetW() * 0.5f, GetH() * 0.5f, 0);
		auto v = glm::vec3(param.x - l.x, param.y - l.y, 0);
		if (glm::length(v) < 50)
		{
			_speed = 0.0f;
			_axis = glm::vec3(0, 0, 0);
		}
		else
		{
			_axis = glm::cross(v, glm::vec3(0, 0, 1));
			_axis = glm::normalize(_axis);
			_axis.x = -_axis.x;

			auto s = glm::length(v) / glm::length(l);
			_speed = s * AppWindow::s_MAX_SPEED;
		}
	}
	
	void OnTimerUpdate()
	{
		auto camera = mmc::mRender.GetCamera(0);
		if (_axis.x != 0 || _axis.y != 0 || _axis.z != 0)
		{
			camera->SetEye(glm::quat(glm::angleAxis(_speed, _axis)) * camera->GetEye());
		}
		if (_direct != 0)
		{
			auto pos = camera->GetPos();
			if ((_direct & kFRONT) != 0)
			{
				pos += camera->GetEye();
			}
			if ((_direct & kBACK) != 0)
			{
				pos -= camera->GetEye();
			}
			if ((_direct & kUP) != 0)
			{
				pos.y += 1;
			}
			if ((_direct & kDOWN) != 0)
			{
				pos.y -= 1;
			}
			if ((_direct & kLEFT) != 0)
			{
				pos += glm::cross(camera->GetUp(), camera->GetEye());
			}
			if ((_direct & kRIGHT) != 0)
			{
				pos -= glm::cross(camera->GetUp(), camera->GetEye());
			}
			camera->SetPos(pos);
		}
		mmc::mTimer.Add(16, std::bind(&AppWindow::OnTimerUpdate, this));
	}
	
private:
	glm::vec3 _axis;
	float _speed;
	int _direct;
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