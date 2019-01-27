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
			glm::vec3(0, 3, 3),
			glm::vec3(0, 0, 0),
			glm::vec3(0, 1, 0));
		mmc::mRender.AddCamera(0, camera);
	}

	void InitAssets()
	{
		File::LoadShader("res/fbo/normal.shader");
		File::LoadModel("res/alpha/floor.obj");
		File::LoadModel("res/alpha/box.obj");
		//	地板
		File::LoadBitmap("res/alpha/floor.png");
		//	箱子
		File::LoadBitmap("res/bitmap/container2.png");
	}

	void InitObject()
	{
		//	构建地板
		Material materialFloor;
		materialFloor.mDiffuses.push_back(File::LoadTexture("res/alpha/floor.png"));
		auto spriteFloor = new Sprite();
		auto modelFloor = File::LoadModel("res/alpha/floor.obj");
		spriteFloor->SetShader(File::LoadShader("res/fbo/normal.shader"));
		spriteFloor->AddMesh(modelFloor->mChilds.at(0)->mMeshs.at(0), materialFloor);
		auto objectFloor = new Object();
		objectFloor->AddComponent(spriteFloor);
		objectFloor->GetTransform()->Scale(5);
		objectFloor->SetParent(&mmc::mRoot);

		//	构建箱子
		Material materialBox;
		materialBox.mDiffuses.push_back(File::LoadTexture("res/bitmap/container2.png"));
		auto spriteBox = new Sprite();
		auto modelBox = File::LoadModel("res/alpha/box.obj");
		spriteBox->SetShader(File::LoadShader("res/fbo/normal.shader"));
		spriteBox->AddMesh(modelBox->mChilds.at(0)->mMeshs.at(0), materialBox);
		auto objectBox = new Object();
		objectBox->AddComponent(spriteBox);
		objectBox->GetTransform()->Translate(0.0f, 0.5f, 0.0f);
		objectBox->SetParent(&mmc::mRoot);

		//	构建RT
		auto renderTarget = new RenderTarget(GetW(), GetH());
		auto objectRT = new Object();
		objectRT->AddComponent(renderTarget);
		objectRT->SetParent(&mmc::mRoot);
		renderTarget->Beg();
		objectFloor->Update(0);
		objectBox->Update(0);
		mmc::mRender.RenderOnce();
		renderTarget->End();
		
		objectBox->DelThis();
		objectFloor->DelThis();

		//	构建后期处理画布
		Material materialPost;
		materialPost.mDiffuses.push_back(renderTarget->GetColorTex(true));
		auto spritePost = new Sprite();
		auto modelPost = File::LoadModel("res/alpha/floor.obj");
		spritePost->SetFlipUVY(true);
		spritePost->SetShader(File::LoadShader("res/fbo/1.shader"));
		spritePost->AddMesh(modelPost->mChilds.at(0)->mMeshs.at(0), materialPost);
		auto objectPost = new Object();
		objectPost->AddComponent(spritePost);
		objectPost->GetTransform()->Scale(5);
		objectPost->SetParent(&mmc::mRoot);
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