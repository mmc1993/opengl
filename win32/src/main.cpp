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
		camera->Init(60, (float)GetW(), (float)GetH(), 0.1f, 500);
		camera->LookAt(
			glm::vec3(0, 0, 3),
			glm::vec3(0, 0, 0),
			glm::vec3(0, 1, 0));
		mmc::mRender.AddCamera(0, camera);
	}

	void InitAssets()
	{
	}

	void InitObject()
	{
		auto modelPlanet = File::LoadModel("res/instance/planet/planet.obj");
		auto spritePlanet = new Sprite();
		spritePlanet->AddMesh(modelPlanet->mChilds.at(0)->mMeshs.at(0), 
							  modelPlanet->mChilds.at(0)->mMaterials.at(0));
		spritePlanet->SetShader(File::LoadShader("res/alpha/normal.shader"));
		auto objectPlanet = new Object();
		objectPlanet->AddComponent(spritePlanet);
		objectPlanet->GetTransform()->Translate(0, 0, -5);
		objectPlanet->SetParent(&mmc::mRoot);



		//	修改顶点属性，把实例坐标塞进去
		const auto rockCount = 1000;
		std::vector<glm::vec3> points;
		for (auto i = 0.0f; i <= 1.0f; i += 1.0f / rockCount)
		{
			auto radius = (float)M_PI * 2.0f * i;
			auto x = std::cos(radius) * 1000.0f + std::rand() % 200 * 0.01f - 1.0f;
			auto z = std::sin(radius) * 1000.0f + std::rand() % 200 * 0.01f - 1.0f;
			auto y = std::rand() % 10000 * 0.01f - 5.0f;
			points.emplace_back(x, y, z);
		}

		auto modelRock = File::LoadModel("res/instance/rock/rock.obj");
		auto spriteRock = new SpriteBatch();
		spriteRock->AddMesh(modelRock->mChilds.at(0)->mMeshs.at(0),
							modelRock->mChilds.at(0)->mMaterials.at(0));
		spriteRock->SetShader(File::LoadShader("res/instance/normal.shader"));
		spriteRock->SetCount(rockCount);
		auto meshRock = modelRock->mChilds.at(0)->mMeshs.at(0);
		glBindVertexArray(meshRock->GetGLID());
		glBufferData(GL_ARRAY_BUFFER, sizeof(Mesh::Vertex) * meshRock->GetVertexs().size() + sizeof(glm::vec3) * points.size(), nullptr, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Mesh::Vertex) * meshRock->GetVertexs().size(), meshRock->GetVertexs().data());
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(Mesh::Vertex) * meshRock->GetVertexs().size(), sizeof(glm::vec3) * points.size(), points.data());
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const void *)(sizeof(Mesh::Vertex) * meshRock->GetVertexs().size()));
		glEnableVertexAttribArray(5);
		glVertexAttribDivisor(5, 1);
		glBindVertexArray(0);

		auto objectRock = new Object();
		objectRock->AddComponent(spriteRock);
		objectRock->GetTransform()->Scale(0.02f);
		objectRock->GetTransform()->Translate(0, 0.5f, -5);
		objectRock->SetParent(&mmc::mRoot);
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