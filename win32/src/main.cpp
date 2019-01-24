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
			glm::vec3(0.0f,		0.0f,	0.0f),
			glm::vec3(2.0f,		5.0f,	-15.0f),
			glm::vec3(-1.5f,	-2.2f,	-2.5f),
			glm::vec3(-3.8f,	-2.0f,	-12.3f),
			glm::vec3(2.4f,		-0.4f,	-3.5f),
			glm::vec3(-1.7f,	3.0f,	-7.5f),
			glm::vec3(1.3f,		-2.0f,	-2.5f),
			glm::vec3(1.5f,		2.0f,	-2.5f),
			glm::vec3(1.5f,		0.2f,	-1.5f),
			glm::vec3(-1.3f,	1.0f,	-1.5f)
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
		//	坐标，环境光，漫反射，镜面反射，方向
		const glm::vec3 directs[][5] = {
			{ glm::vec3(0, 5, 0), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.3f, 0.3f, 0.3f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0, -1, 0) },
		};

		//	坐标，环境光，漫反射，镜面反射，衰减k0, k1, k2
		const glm::vec3 points[][5] = {
			{ glm::vec3(0, 0, -5), glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.4f, 0.4f, 0.4f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.09f, 0.05f) },
		};

		//	坐标，环境，漫反射，镜面反射，方向，衰减k0, k1, k2，内切角，外切角
		const glm::vec3 spots[][7] = {
			{ glm::vec3(0, 0, 3), glm::vec3(0.3f, 0.3f, 0.3f), glm::vec3(0.3f, 0.3f, 0.3f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f, 0.01f, 0.1f), glm::vec3(0.99f, 0.90f, 0.0f) },
		};

		for (auto & data : directs)
		{
			auto light = new LightDirect();
			light->mMesh = mmc::mAssetCore.Get<Mesh>("res/model/1/model.obj");
			light->mShader = mmc::mAssetCore.Get<Shader>("res/shader/light.shader");
			light->mAmbient = data[1];
			light->mDiffuse = data[2];
			light->mSpecular = data[3];
			light->mNormal = data[4];
			auto object = new Object();
			object->AddComponent(light);
			object->GetTransform()->Translate(data[0]);
			object->SetParent(&mmc::mRoot);
		}

		for (auto & data : points)
		{
			auto light = new LightPoint();
			light->mMesh = mmc::mAssetCore.Get<Mesh>("res/model/1/model.obj");
			light->mShader = mmc::mAssetCore.Get<Shader>("res/shader/light.shader");
			light->mAmbient = data[1];
			light->mDiffuse = data[2];
			light->mSpecular = data[3];
			light->mK0 = data[4].x;
			light->mK1 = data[4].y;
			light->mK2 = data[4].z;
			auto object = new Object();
			object->AddComponent(light);
			object->GetTransform()->Translate(data[0]);
			object->SetParent(&mmc::mRoot);
		}
		
		for (auto & data : spots)
		{
			auto light = new LightSpot();
			light->mMesh = mmc::mAssetCore.Get<Mesh>("res/model/1/model.obj");
			light->mShader = mmc::mAssetCore.Get<Shader>("res/shader/light.shader");
			light->mAmbient = data[1];
			light->mDiffuse = data[2];
			light->mSpecular = data[3];
			light->mNormal = data[4];
			light->mK0 = data[5].x;
			light->mK1 = data[5].y;
			light->mK2 = data[5].z;
			light->mInCone = data[6].x;
			light->mOutCone = data[6].y;
			auto object = new Object();
			object->AddComponent(light);
			object->GetTransform()->Translate(data[0]);
			object->SetParent(&mmc::mRoot);
		}
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
		auto l = glm::vec2(GetW() * 0.5f, GetH() * 0.5f);
		auto v = glm::vec2(param.x - l.x, l.y - param.y);
		if (glm::length(v) < 100)
		{
			_speed = 0.0f;
			_axis.x = 0;
			_axis.y = 0;
			_axis.z = 0;
		}
		else
		{
			auto camera = mmc::mRender.GetCamera(0);
			auto a = glm::dot(glm::vec2(1, 0), glm::normalize(v));
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
				pos += camera->GetEye() * 0.1f;
			}
			if ((_direct & kBACK) != 0)
			{
				pos -= camera->GetEye() * 0.1f;
			}
			if ((_direct & kUP) != 0)
			{
				pos.y += 1 * 0.1f;
			}
			if ((_direct & kDOWN) != 0)
			{
				pos.y -= 1 * 0.1f;
			}
			if ((_direct & kLEFT) != 0)
			{
				pos += glm::cross(camera->GetUp(), camera->GetEye()) * 0.1f;
			}
			if ((_direct & kRIGHT) != 0)
			{
				pos -= glm::cross(camera->GetUp(), camera->GetEye()) * 0.1f;
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