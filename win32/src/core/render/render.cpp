#include "render.h"
#include "../mmc.h"
#include "../asset/shader.h"
#include "../object/camera.h"
#include "../tools/debug_tool.h"

Render::Render()
{ }

Render::~Render()
{ }

void Render::AddCamera(size_t id, ::Camera * camera)
{
    assert(std::count(_cameras.begin(), _cameras.end(), id) == 0);
    auto it = std::lower_bound(
        _cameras.begin(), 
        _cameras.end(), id);
    _cameras.insert(it, Camera(camera, id));
}

void Render::DelCamera(size_t id)
{
    auto it = std::find(_cameras.begin(), _cameras.end(), id);
    if (it != _cameras.end())
    {
        _cameras.erase(it);
    }
}

void Render::Bind(Mesh * mesh)
{
	_renderInfo.mMesh = mesh;
}

void Render::Bind(Shader * shader)
{
	_renderInfo.mShader = shader;
}

void Render::Bind(Material * material)
{
	_renderInfo.mMaterial = material;
}

void Render::PostCommand(const Command & command)
{
    assert(command.mCallFn != nullptr);
	_commands.push_back(command);
}

Render::Matrix & Render::GetMatrix()
{
	return _matrix;
}

Render::Light & Render::GetLight()
{
	return _light;
}

void Render::RenderMesh(size_t count)
{
	assert(_renderInfo.mShader != nullptr);
	_light.Bind(_renderInfo.mShader->GetGLID());
	glDrawArrays(GL_TRIANGLES, 0, count);
}

void Render::RenderOnce()
{
    for (auto & camera : _cameras)
    {
		camera.mCamera->Bind();
		OnRenderCamera(camera);
		camera.mCamera->Free();
    }
	_commands.clear();
}

void Render::OnRenderCamera(Camera & camera)
{
    for (auto & command : _commands)
    {
        if (command.mCameraID == camera.mID)
        {
			command.mCallFn();
		}
    }
}

void Render::CommandTransform::Post(size_t cameraID, const glm::mat4 & mat)
{
	Command command;
	command.mCameraID = cameraID;
	command.mCallFn = [mat]() {
		mmc::mRender.GetMatrix().Push(Render::Matrix::kMODELVIEW);
		mmc::mRender.GetMatrix().Mul(Render::Matrix::kMODELVIEW, mat);
	};
	mmc::mRender.PostCommand(command);
}

void Render::CommandTransform::Free(size_t cameraID)
{
	Command command;
	command.mCameraID = cameraID;
	command.mCallFn = []() {
		mmc::mRender.GetMatrix().Pop(Render::Matrix::kMODELVIEW);
	};
	mmc::mRender.PostCommand(command);
}

void Render::Light::SetAmbient(float value)
{
	_ambient = value;
}

void Render::Light::Add(::Light * light)
{
	assert(light != nullptr);
	CHECK_RET(CheckCount(light));
	switch (light->GetType())
	{
	case ::Light::kPOINT:
		{
			auto point = dynamic_cast<LightPoint *>(light);
			assert(point != nullptr);
			assert(std::count(_points.begin(), _points.end(), point) == 0);
			_points.push_back(point);
			_change = true;
		}
		break;
	case ::Light::kSPOT:
		{
			auto spot = dynamic_cast<LightSpot *>(light);
			assert(spot != nullptr);
			assert(std::count(_spots.begin(), _spots.end(), spot) == 0);
			_spots.push_back(spot);
			_change = true;
		}
		break;
	default: assert(false); break;
	}
}

void Render::Light::Del(::Light * light)
{
	assert(light != nullptr);
	switch (light->GetType())
	{
	case ::Light::kPOINT:
		{
			auto it = std::remove(_points.begin(), _points.end(), light);
			if (it != _points.end())
			{
				_points.erase(it);
				_change = true;
			}
		}
		break;
	case ::Light::kSPOT:
		{
			auto it = std::remove(_spots.begin(), _spots.end(), light);
			if (it != _spots.end())
			{
				_spots.erase(it);
				_change = true;
			}
		}
		break;
	}
}

void Render::Light::Bind(GLuint shaderID)
{
	Update();
	auto index = glGetUniformBlockIndex(shaderID, "light_");
	glUniformBlockBinding(shaderID, index, GL_UBO_IDX::kLIGHT);
}

float Render::Light::GetAmbient() const
{
	return _ambient;
}

const std::vector<::LightPoint*>& Render::Light::GetPoints() const
{
	return _points;
}

const std::vector<::LightSpot*>& Render::Light::GetSpots() const
{
	return _spots;
}

void Render::Light::Update()
{
	if (_GLID == 0)
	{
		glGenBuffers(1, &_GLID);
	}
	if (_change)
	{
		_change = false;
		auto ubo = std::make_unique<UBO>();
		ubo->mAmbient = GetAmbient();
		ubo->mSpotNum = static_cast<short>(_spots.size());
		ubo->mPointNum = static_cast<short>(_points.size());
		for (auto i = 0; i != _points.size(); ++i)
		{
			ubo->mPoints[i] = _points.at(i)->GetValue();
		}
		for (auto i = 0; i != _spots.size(); ++i)
		{
			ubo->mSpots[i] = _spots.at(i)->GetValue();
		}
		assert(_GLID != 0);
		glBindBuffer(GL_UNIFORM_BUFFER, _GLID);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(UBO), ubo.get(), GL_DYNAMIC_DRAW);
		glBindBufferRange(GL_UNIFORM_BUFFER, GL_UBO_IDX::kLIGHT, _GLID, 0, sizeof(UBO));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
}

bool Render::Light::CheckCount(::Light * light) const
{
	switch (light->GetType())
	{
	case ::Light::kPOINT: return _points.size() != Light::MAX_POINT;
	case ::Light::kSPOT: return _spots.size() != Light::MAX_SPOT;
	}
	assert(false);
	return false;
}
