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

void Render::Bind(::Camera * camera)
{
	_renderInfo.mCamera = camera;
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
	_renderInfo.mShader->SetUniform("nmvp_", GetMatrix().GetNMat());
	_renderInfo.mShader->SetUniform("mvp_", GetMatrix().GetMVP());
	_renderInfo.mShader->SetUniform("mv_", GetMatrix().GetMV());
	_renderInfo.mShader->SetUniform("camera_pos_", _renderInfo.mCamera->GetPos());
	_renderInfo.mShader->SetUniform("camera_eye_", _renderInfo.mCamera->GetEye());
	glDrawArrays(GL_TRIANGLES, 0, count);
}

void Render::RenderOnce()
{
    for (auto & camera : _cameras)
    {
		Bind(camera.mCamera);
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
	Update(shaderID);
	auto index = glGetUniformBlockIndex(shaderID, "Light_");
	glUniformBlockBinding(shaderID, index, GL_UBO_IDX::kLIGHT);
}

void Render::Light::Update(GLuint shaderID)
{
	if (_change)
	{
		_change = false;
		if (_GLID == 0)
		{
			GLint bufferLength = 0;
			glGetActiveUniformBlockiv(shaderID, GL_UBO_IDX::kLIGHT,
						GL_UNIFORM_BLOCK_DATA_SIZE, &bufferLength);
			glGenBuffers(1, &_GLID);
			glBindBuffer(GL_UNIFORM_BUFFER, _GLID);
			glBufferData(GL_UNIFORM_BUFFER, bufferLength, nullptr, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_UNIFORM_BUFFER, GL_UBO_IDX::kLIGHT, _GLID);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		auto ubo = std::make_unique<UBO>();
		ubo->mAmbient = _ambient;
		ubo->mPointNum = static_cast<int>(_points.size());
		ubo->mSpotNum = static_cast<int>(_spots.size());
		for (auto i = 0; i != _points.size(); ++i)
		{ 
			ubo->mPoints[i].mPos = glm::vec4(_points.at(i)->GetValue().mPos, 1);
			ubo->mPoints[i].mColor = _points.at(i)->GetValue().mColor;
			ubo->mPoints[i].mMin = _points.at(i)->GetValue().mMin;
			ubo->mPoints[i].mMax = _points.at(i)->GetValue().mMax;
		}
		for (auto i = 0; i != _spots.size(); ++i)
		{ 
			ubo->mSpots[i].mPos = glm::vec4(_spots.at(i)->GetValue().mPos, 1);
			ubo->mSpots[i].mDir = glm::vec4(_spots.at(i)->GetValue().mPos, 1);
			ubo->mSpots[i].mColor = _spots.at(i)->GetValue().mColor;
			ubo->mSpots[i].mMinCone = _spots.at(i)->GetValue().mMinCone;
			ubo->mSpots[i].mMaxCone = _spots.at(i)->GetValue().mMaxCone;
			ubo->mSpots[i].mMin = _spots.at(i)->GetValue().mMin;
			ubo->mSpots[i].mMax = _spots.at(i)->GetValue().mMax;
		}
		
		const char * uboNames[] = {
			"Light_.mAmbient", 
			"Light_.mPointNum",
			"Light_.mSpotNum",
			"Light_.mPoints[0].mPos",
			"Light_.mPoints[0].mColor",
			"Light_.mPoints[0].mMin",
			"Light_.mPoints[0].mMax",
			"Light_.mPoints[1].mPos",
			"Light_.mPoints[1].mColor",
			"Light_.mPoints[1].mMin",
			"Light_.mPoints[1].mMax",
			"Light_.mPoints[2].mPos",
			"Light_.mPoints[2].mColor",
			"Light_.mPoints[2].mMin",
			"Light_.mPoints[2].mMax",
			//"Light_.mSpots",
		};
		
		GLuint indices[sizeof(uboNames) / sizeof(char *)] = { 0 };
		GLint offsets[sizeof(uboNames) / sizeof(char *)] = { 0 };
		glGetUniformIndices(shaderID, sizeof(uboNames) / sizeof(char *), uboNames, indices);
		glGetActiveUniformsiv(shaderID, sizeof(uboNames) / sizeof(char *), indices, GL_UNIFORM_OFFSET, offsets);

		//glBindBuffer(GL_UNIFORM_BUFFER, _GLID);
		//const GLchar * names[] = {
		//	"UBO.xxx.aaa"
		//};
		//GLuint indices[1] = { 0 };
		//glGetUniformIndices(shaderID, 1, names, indices);
		//GLint size[1] = { 0 };
		//glGetActiveUniformsiv(shaderID, 1, indices, GL_UNIFORM_SIZE, size);

		glBindBuffer(GL_UNIFORM_BUFFER, _GLID);
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBO, mAmbient), sizeof(GLfloat), &ubo->mAmbient);
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBO, mPointNum), sizeof(UBO::mPointNum), &ubo->mPointNum);
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBO, mSpotNum), sizeof(UBO::mSpotNum), &ubo->mSpotNum);
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBO, mPoints), sizeof(UBO::mPoints), &ubo->mPoints);
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(UBO, mSpots), sizeof(UBO::mSpots), &ubo->mSpots);
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
