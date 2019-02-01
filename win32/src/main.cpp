#include "core/mmc.h"
#include "core/timer/timer.h"
#include "core/event/event.h"
#include "core/window/window.h"
#include "core/object/camera.h"
#include "core/render/render.h"
#include "core/asset/shader.h"
#include "core/asset/material.h"
#include "core/component/sprite.h"
#include "core/component/render_target.h"
#include "core/component/sprite_batch.h"
#include "core/asset/asset_core.h"
#include "core/tools/debug_tool.h"
#include "core/component/light.h"
#include "core/component/transform.h"
#include "core/component/skybox.h"
#include "core/asset/model.h"
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
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		InitCamera();
		InitAssets();
		InitEvents();
		InitLights();
		InitObject();
    }
private:
	void InitCamera()
	{
		auto camera = new Camera();
		camera->InitPerspective(60, (float)GetW(), (float)GetH(), 0.1f, 500);
		camera->SetViewport({ 0, 0, GetW(), GetH() });
		camera->LookAt(
			glm::vec3(0, 5, 5),
			glm::vec3(0, 0, 0),
			glm::vec3(0, 1, 0));
		mmc::mRender.AddCamera(0, camera, 0);
		
		auto camera2 = new Camera();
		camera2->InitOrthogonal(-5.0f, 5.0f, -5.0f, 5.0f, -10.0f, 1000.0f);
		camera2->SetViewport({ 0, 0, GetW() * 0.5f, GetH() * 0.5f });
		camera2->LookAt({ -5, 100, 0 }, { 0, 0, 0 }, { 0, 0, -1 });
		mmc::mRender.AddCamera(0, camera2, 1);
	}

	void InitAssets()
	{
	}

	void InitObject()
	{
		auto modelFloor = File::LoadModel("res/shadow/floor.obj");
		modelFloor->mChilds.at(0)->mMaterials.at(0).mDiffuses.push_back(File::LoadTexture("res/shadow/wood.png"));

		auto modelBox = File::LoadModel("res/shadow/box.obj");
		modelBox->mChilds.at(0)->mMaterials.at(0).mDiffuses.push_back(File::LoadTexture("res/shadow/container_diffuse.png"));
		modelBox->mChilds.at(0)->mMaterials.at(0).mSpeculars.push_back(File::LoadTexture("res/shadow/container_specular.png"));

		//	地板
		auto spriteFloor = new Sprite();
		spriteFloor->SetShader(File::LoadShader("res/shadow/shadow.shader"));
		spriteFloor->AddMesh(modelFloor->mChilds.at(0)->mMeshs.at(0), modelFloor->mChilds.at(0)->mMaterials.at(0));
		auto objectFloor = new Object();
		objectFloor->AddComponent(spriteFloor);
		objectFloor->GetTransform()->Scale(10, 0.1f, 10);
		objectFloor->SetParent(&mmc::mRoot);

		//	箱子
		auto spriteBox = new Sprite();
		spriteBox->SetShader(File::LoadShader("res/shadow/box.shader"));
		spriteBox->AddMesh(modelBox->mChilds.at(0)->mMeshs.at(0), modelBox->mChilds.at(0)->mMaterials.at(0));
		auto objectBox = new Object();
		objectBox->AddComponent(spriteBox);
		objectBox->GetTransform()->Translate(0, 1, 0);
		objectBox->SetParent(&mmc::mRoot);

		_lightSpots.at(0)->OpenShadow(512, 512, 1, 1000, glm::vec3(0, 0, -1));
		_lightSpots.at(1)->OpenShadow(512, 512, 1, 1000, glm::vec3(0, 0, -1));

		//auto shadowRT = _lightSpots.at(0)->DrawShadow(false);

		//modelFloor->mChilds.at(0)->mMaterials.at(0).mDiffuses.at(0) = Texture(shadowRT->GetDepthTex());

		//auto spriteShadow = new Sprite();
		//spriteShadow->SetShader(File::LoadShader("res/shadow/depth.shader"));
		//spriteShadow->AddMesh(modelFloor->mChilds.at(0)->mMeshs.at(0), modelFloor->mChilds.at(0)->mMaterials.at(0));
		//auto objectShadow = new Object();
		//objectShadow->AddComponent(spriteShadow);
		//objectShadow->GetTransform()->Translate(2, 1, 0);
		//objectShadow->SetParent(&mmc::mRoot);
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
		static auto OPEN_DRAW = false;

		//	坐标，环境光，漫反射，镜面反射，方向
		const std::vector<std::array<glm::vec3, 5>> directs = {
			//{ glm::vec3(20, 100, -20), glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.3f, 0.3f, 0.3f), glm::vec3(0.5f, 0.5f, 0.5f), glm::normalize(glm::vec3(0, 0, 0) - glm::vec3(20, 100, -20)) },
			//{ glm::vec3(-20, 100, 20), glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.3f, 0.3f, 0.3f), glm::vec3(0.5f, 0.5f, 0.5f), glm::normalize(glm::vec3(0, 0, 0) - glm::vec3(-20, 100, 20)) },
		};

		//	坐标，环境光，漫反射，镜面反射，衰减k0, k1, k2
		const std::vector<std::array<glm::vec3, 5>> points = {
			//{ glm::vec3(0, 5, 0), glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.6f, 0.6f, 0.6f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.01f, 0.01f) },
		};

		//	坐标，环境，漫反射，镜面反射，方向，衰减k0, k1, k2，内切角，外切角
		const std::vector<std::array<glm::vec3, 7>> spots = {
			{ glm::vec3(2, 5, 0), glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.7f, 0.7f, 0.7f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0, -1, 0), glm::vec3(1.0f, 0.01f, 0.01f), glm::vec3(0.9f, 0.8f, 0.0f) },
			{ glm::vec3(-2, 5, 0), glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.7f, 0.7f, 0.7f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0, -1, 0), glm::vec3(1.0f, 0.01f, 0.01f), glm::vec3(0.9f, 0.8f, 0.0f) },
		};

		for (auto & data : directs)
		{
			auto light = new LightDirect();
			light->mIsDraw = OPEN_DRAW;
			light->mAmbient = data[1];
			light->mDiffuse = data[2];
			light->mSpecular = data[3];
			light->mNormal = data[4];
			auto object = new Object();
			object->AddComponent(light);
			object->GetTransform()->Translate(data[0]);
			object->SetParent(&mmc::mRoot);
			_lightDirects.push_back(light);
		}

		for (auto & data : points)
		{
			auto light = new LightPoint();
			light->mIsDraw = OPEN_DRAW;
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
			_lightPoints.push_back(light);
		}

		for (auto & data : spots)
		{
			auto light = new LightSpot();
			light->mIsDraw = OPEN_DRAW;
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
			_lightSpots.push_back(light);
		}

		_val = 0;
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
			_axis.x = _axis.y = _axis.z = _speed = 0.0f;
		}
		else
		{
			auto camera = mmc::mRender.GetCamera(0);
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
			if ((_direct & kUP) != 0) { pos.y += 1 * 0.1f; }
			if ((_direct & kDOWN) != 0) { pos.y -= 1 * 0.1f; }
			if ((_direct & kFRONT) != 0) { pos += camera->GetEye() * 0.1f; }
			if ((_direct & kBACK) != 0) { pos -= camera->GetEye() * 0.1f; }
			if ((_direct & kLEFT) != 0) { pos += glm::cross(camera->GetUp(), camera->GetEye()) * 0.1f; }
			if ((_direct & kRIGHT) != 0) { pos -= glm::cross(camera->GetUp(), camera->GetEye()) * 0.1f; }
			camera->SetPos(pos);
		}

		auto onwer = _lightSpots.at(0)->GetOwner();
		auto pos = onwer->GetTransform()->GetPosition();
		pos.x = std::cos(_val) * 5.0f;
		pos.z = std::sin(_val) * 5.0f;
		pos.y = 5.0f;
		onwer->GetTransform()->Translate(pos);
		_lightSpots.at(0)->mNormal = glm::normalize(glm::vec3(0) - pos);
		_val += 0.01f;

		mmc::mTimer.Add(16, std::bind(&AppWindow::OnTimerUpdate, this));
	}
	
private:
	std::vector<LightDirect *> _lightDirects;
	std::vector<LightPoint *> _lightPoints;
	std::vector<LightSpot *> _lightSpots;
	glm::vec3 _axis;
	float _speed;
	int _direct;

	float _val;
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