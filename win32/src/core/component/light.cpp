#include "light.h"
#include "transform.h"
#include "../res/shader.h"
#include "../render/render.h"
#include "../cfg/cfg_cache.h"
#include "../res/res_cache.h"
#include "../tools/glsl_tool.h"

std::weak_ptr<Mesh> Light::s_spotVolmue;
std::weak_ptr<Mesh> Light::s_pointVolmue;
std::weak_ptr<Mesh> Light::s_directVolmue;

std::shared_ptr<Mesh> Light::NewVolume()
{
    switch (_type)
    {
    case Type::kDIRECT:
        {
            if (!s_directVolmue.expired())
            {
                auto windowW = Global::Ref().RefCfgCache().At("init")->At("window", "w")->ToInt();
                auto windowH = Global::Ref().RefCfgCache().At("init")->At("window", "h")->ToInt();
                auto mesh = Mesh::CreateV({ { { -1.0f, -1.0f, 0.0f } }, { {  1.0f, -1.0f, 0.0f } },
                                            { {  1.0f,  1.0f, 0.0f } }, { { -1.0f,  1.0f, 0.0f } } }, { 0, 1, 2, 0, 2, 3 });
                std::shared_ptr<Mesh> sharePtr(new Mesh(), Mesh::DeletePtr);
                s_directVolmue = sharePtr; *sharePtr = mesh;
                return s_directVolmue.lock();
            }
            return s_directVolmue.lock();
        }
        break;
    case Type::kPOINT:
        {
            if (!s_pointVolmue.expired())
            {
                const auto N0 = 4;
                const auto N1 = N0 * 2-2;
                std::vector<uint> indexs;
                std::vector<Mesh::Vertex> vertexs;

                vertexs.emplace_back(glm::vec3(0, 1, 0));
                for (auto i = 0; i != N1; ++i)
                {
                    indexs.emplace_back(0);
                    indexs.emplace_back((i    ) % N1 + 1);
                    indexs.emplace_back((i + 1) % N1 + 1);
                }

                auto step = static_cast<float>(M_PI / N0);
                for (auto i = 1; i != N0 - 1; ++i)
                {
                    auto a = step * i;
                    auto x = std::sin(a);
                    auto y = std::cos(a);
                    auto base = (i - 1) * N1;
                    for (auto j = 0; j != N1; ++j)
                    {
                        if (i != 1 && i != N0 - 2)
                        {
                            auto point0 = (j + 1);
                            auto point1 = (j + 1) % N1 + 1;
                            indexs.push_back(base - N1 + point0);
                            indexs.push_back(point0);
                            indexs.push_back(point1);

                            indexs.push_back(base - N1 + point0);
                            indexs.push_back(point1);
                            indexs.push_back(base - N1 + point1);
                        }
                        vertexs.emplace_back(glm::vec3(x, y, std::cos(step * j)));
                    }
                }

                auto base = (N0 - 1) * N1 + 1;
                for (auto i = 0; i != N1; ++i)
                {
                    auto point0 =  i;
                    auto point1 = (i + 1) % N1;
                    indexs.emplace_back(base + point0);
                    indexs.emplace_back(vertexs.size());
                    indexs.emplace_back(base + point1);
                }

                vertexs.emplace_back(glm::vec3(0, -1, 0));

                std::shared_ptr<Mesh> sharePtr(new Mesh(), &Mesh::DeletePtr);
                *sharePtr = Mesh::CreateV(vertexs, indexs); 
                s_pointVolmue = sharePtr;
                return s_pointVolmue.lock();
            }
            return s_pointVolmue.lock();
        }
        break;
    case Type::kSPOT:
        {
            if (s_spotVolmue.expired())
            {
                const auto N = 6;
                std::vector<uint> indexs;
                std::vector<Mesh::Vertex> vertexs;

                vertexs.emplace_back(glm::vec3(0, 0, 0));

                auto step = static_cast<float>(M_PI / N);
                for (auto i = 0; i != N; ++i)
                {
                    auto a = step * i;
                    auto x = std::sin(a);
                    auto y = std::cos(a);
                    vertexs.emplace_back(glm::vec3(x, y, 0.5f));

                    indexs.emplace_back(0);
                    indexs.emplace_back( i + 1);
                    indexs.emplace_back((i + 1) % N + 1);

                    indexs.emplace_back(1);
                    indexs.emplace_back( i + 2);
                    indexs.emplace_back((i + 2) % N + 1);
                }

                std::shared_ptr<Mesh> sharePtr(new Mesh(), &Mesh::DeletePtr);
                *sharePtr = Mesh::CreateV(vertexs, indexs);
                s_spotVolmue = sharePtr;
                return s_spotVolmue.lock();
            }
            return s_spotVolmue.lock();
        }
        break;
    }
    return nullptr;
}

Light::Light(Type type): _type(type), _shadowMap(0), _uniformBlock(0)
{
    _volume = NewVolume();
    _shader = Global::Ref().RefResCache().Get<Shader>(BUILTIN_SHADER_LIGHT);
}

//  --------------------------------------------------------------------------------
//  光源实现
void Light::OnUpdate(float dt)
{
    LightCommand command;
    command.mLight      = this;
    command.mMesh       = _volume.get();
    command.mTransform  = Global::Ref().RefRender().GetMatrixStack().GetM();
    Global::Ref().RefRender().PostCommand(_shader, command);
}

float Light::CalLightDistance(float k0, float k1, float k2, float s)
{
    auto a = k2 * s;
    auto b = k1 * s - 1;
    auto c = k0 * s;
    return (-b + std::sqrt(b * b - 4 * a * c)) / (2 * a);
}

uint LightDirect::GetUBOLength()
{
    auto base = glsl_tool::UBOOffsetFill<decltype(UBOData::mMatrix)>(0);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mNormal)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mAmbient)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mDiffuse)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mSpecular)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mPosition)>(base);
    return glsl_tool::UBOOffsetBase<glm::vec4>(base);
}

void LightDirect::OpenShadow(
	const glm::vec2 & orthoX,
	const glm::vec2 & orthoY,
	const glm::vec2 & orthoZ)
{
    if (GetUBO() == 0)
    {
        glGenBuffers(1, &_uniformBlock);
        glBindBuffer(GL_UNIFORM_BUFFER, GetUBO());
        glBufferData(GL_UNIFORM_BUFFER, GetUBOLength(), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    if (GetSMP() == 0)
    {
        auto viewW = Global::Ref().RefCfgCache().At("init")->At("shadow_map", "w")->ToInt();
        auto viewH = Global::Ref().RefCfgCache().At("init")->At("shadow_map", "h")->ToInt();

        glGenTextures(1, &_shadowMap);
        glBindTexture(GL_TEXTURE_2D, _shadowMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, viewW, viewH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    _proj = glm::ortho(orthoX.x, orthoX.y, orthoY.x, orthoY.y, orthoZ.x, orthoZ.y);
}

bool LightDirect::NextDrawShadow(uint count, RenderTarget * rt)
{
    if (0 == count)
    {
        auto viewW = Global::Ref().RefCfgCache().At("init")->At("shadow_map", "w")->ToInt();
        auto viewH = Global::Ref().RefCfgCache().At("init")->At("shadow_map", "w")->ToInt();

        mPosition   = GetOwner()->GetTransform()->GetWorldPosition();
        auto up     = std::abs(mNormal.y) > 0.999f
                    ? glm::vec3(0, 0, 1) 
                    : glm::vec3(0, 1, 0);
        auto right  = glm::cross(up, mNormal);
        up          = glm::cross(mNormal, right);
        auto view   = glm::lookAt(mPosition, mPosition + mNormal, up);
        mMatrix     = _proj * view;

        glBindBuffer(GL_UNIFORM_BUFFER, GetUBO());
        auto base = glsl_tool::UBOAddData<decltype(UBOData::mMatrix)>(0, mMatrix);
        base = glsl_tool::UBOAddData<decltype(UBOData::mNormal)>(base, mNormal);
        base = glsl_tool::UBOAddData<decltype(UBOData::mAmbient)>(base, mAmbient);
        base = glsl_tool::UBOAddData<decltype(UBOData::mDiffuse)>(base, mDiffuse);
        base = glsl_tool::UBOAddData<decltype(UBOData::mSpecular)>(base, mSpecular);
        base = glsl_tool::UBOAddData<decltype(UBOData::mPosition)>(base, mPosition);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glViewport(0, 0, viewW, viewH);
        Global::Ref().RefRender().GetMatrixStack().Identity(MatrixStack::kVIEW);
        Global::Ref().RefRender().GetMatrixStack().Identity(MatrixStack::kPROJ);
        Global::Ref().RefRender().GetMatrixStack().Mul(MatrixStack::kVIEW, view);
        Global::Ref().RefRender().GetMatrixStack().Mul(MatrixStack::kPROJ, _proj);
        rt->BindAttachment(RenderTarget::AttachmentType::kDEPTH, RenderTarget::TextureType::k2D, GetSMP());
    }
    else
    {
        Global::Ref().RefRender().GetMatrixStack().Pop(MatrixStack::kPROJ);
        Global::Ref().RefRender().GetMatrixStack().Pop(MatrixStack::kVIEW);
    }
    return count == 0;
}

uint LightPoint::GetUBOLength()
{
    auto base = glsl_tool::UBOOffsetFill<decltype(UBOData::mFar)>(0);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mNear)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mK0)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mK1)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mK2)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mAmbient)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mDiffuse)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mSpecular)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mPosition)>(base);
    return glsl_tool::UBOOffsetBase<glm::vec4>(base);
}

void LightPoint::OpenShadow(const float n, const float f)
{
    auto viewW = Global::Ref().RefCfgCache().At("init")->At("shadow_map", "w")->ToInt();
    auto viewH = Global::Ref().RefCfgCache().At("init")->At("shadow_map", "h")->ToInt();

    if (GetUBO() == 0)
    {
        glGenBuffers(1, &_uniformBlock);
        glBindBuffer(GL_UNIFORM_BUFFER, GetUBO());
        glBufferData(GL_UNIFORM_BUFFER, GetUBOLength(), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    if (GetSMP() == 0)
    {
        glGenTextures(1, &_shadowMap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _shadowMap);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_DEPTH_COMPONENT, viewW, viewH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_DEPTH_COMPONENT, viewW, viewH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_DEPTH_COMPONENT, viewW, viewH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_DEPTH_COMPONENT, viewW, viewH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_DEPTH_COMPONENT, viewW, viewH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_DEPTH_COMPONENT, viewW, viewH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_DEPTH_COMPONENT, viewW, viewH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_DEPTH_COMPONENT, viewW, viewH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

	_proj = glm::perspective(glm::radians(90.0f), (float)viewW / (float)viewH, n, f);
    mFar = f; mNear = n;
}

bool LightPoint::NextDrawShadow(uint count, RenderTarget * rt)
{
    if (count != 0)
    {
        Global::Ref().RefRender().GetMatrixStack().Pop(MatrixStack::kPROJ);
        Global::Ref().RefRender().GetMatrixStack().Pop(MatrixStack::kVIEW);
    }
    else
    {
        auto viewW = Global::Ref().RefCfgCache().At("init")->At("shadow_map", "w")->ToInt();
        auto viewH = Global::Ref().RefCfgCache().At("init")->At("shadow_map", "h")->ToInt();

        glViewport(0, 0, viewW, viewH);

        mPosition = GetOwner()->GetTransform()->GetWorldPosition();

        glBindBuffer(GL_UNIFORM_BUFFER, GetUBO());
        auto base = glsl_tool::UBOAddData<decltype(UBOData::mFar)>(0, mFar);
        base = glsl_tool::UBOAddData<decltype(UBOData::mNear)>(base, mNear);
        base = glsl_tool::UBOAddData<decltype(UBOData::mK0)>(base, mK0);
        base = glsl_tool::UBOAddData<decltype(UBOData::mK1)>(base, mK1);
        base = glsl_tool::UBOAddData<decltype(UBOData::mK2)>(base, mK2);
        base = glsl_tool::UBOAddData<decltype(UBOData::mAmbient)>(base, mAmbient);
        base = glsl_tool::UBOAddData<decltype(UBOData::mDiffuse)>(base, mDiffuse);
        base = glsl_tool::UBOAddData<decltype(UBOData::mSpecular)>(base, mSpecular);
        base = glsl_tool::UBOAddData<decltype(UBOData::mPosition)>(base, mPosition);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    if (count < 6)
    {
        static const std::tuple<glm::vec3, glm::vec3> s_faceInfo[6] = {
            { glm::vec3( 1,  0,  0), glm::vec3(0, -1,  0) },
            { glm::vec3(-1,  0,  0), glm::vec3(0, -1,  0) },
            { glm::vec3( 0,  1,  0), glm::vec3(0,  0,  1) },
            { glm::vec3( 0, -1,  0), glm::vec3(0,  0, -1) },
            { glm::vec3( 0,  0,  1), glm::vec3(0, -1,  0) },
            { glm::vec3( 0,  0, -1), glm::vec3(0, -1,  0) },
        };

        auto view = glm::lookAt(mPosition,
                    std::get<0>(s_faceInfo[count]) + mPosition,
                    std::get<1>(s_faceInfo[count]));

        Global::Ref().RefRender().GetMatrixStack().Identity(MatrixStack::kVIEW);
        Global::Ref().RefRender().GetMatrixStack().Identity(MatrixStack::kPROJ);
        Global::Ref().RefRender().GetMatrixStack().Mul(MatrixStack::kVIEW,  view);
        Global::Ref().RefRender().GetMatrixStack().Mul(MatrixStack::kPROJ, _proj);
        rt->BindAttachment(RenderTarget::AttachmentType::kDEPTH, (RenderTarget::TextureType)(RenderTarget::TextureType::k3D + count), GetSMP());
    }
    return count != 6;
}

uint LightSpot::GetUBOLength()
{
    auto base = glsl_tool::UBOOffsetFill<decltype(UBOData::mK0)>(0);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mK1)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mK2)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mInCone)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mOutCone)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mMatrix)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mNormal)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mAmbient)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mDiffuse)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mSpecular)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mPosition)>(base);
    return glsl_tool::UBOOffsetBase<glm::vec4>(base);
}

void LightSpot::OpenShadow(const float n, const float f)
{
    auto viewW = Global::Ref().RefCfgCache().At("init")->At("shadow_map", "w")->ToInt();
    auto viewH = Global::Ref().RefCfgCache().At("init")->At("shadow_map", "h")->ToInt();

    if (GetUBO() == 0)
    {
        glGenBuffers(1, &_uniformBlock);
        glBindBuffer(GL_UNIFORM_BUFFER, GetUBO());
        glBufferData(GL_UNIFORM_BUFFER, GetUBOLength(), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    if (GetSMP() == 0)
    {
        glGenTextures(1, &_shadowMap);
        glBindTexture(GL_TEXTURE_2D, _shadowMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, viewW, viewH, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    _proj = glm::perspective(glm::radians(90.0f), (float)viewW / (float)viewH, n, f);
}

bool LightSpot::NextDrawShadow(uint count, RenderTarget * rt)
{
    if (count == 0)
    {
        auto viewW = Global::Ref().RefCfgCache().At("init")->At("shadow_map", "w")->ToInt();
        auto viewH = Global::Ref().RefCfgCache().At("init")->At("shadow_map", "h")->ToInt();

        mPosition   = GetOwner()->GetTransform()->GetWorldPosition();
        auto up     = std::abs(mNormal.y) > 0.999f
                    ? glm::vec3(0, 0, 1)
                    : glm::vec3(0, 1, 0);
        auto right  = glm::cross(up, mNormal);
        up          = glm::cross(mNormal, right);
        auto view   = glm::lookAt(mPosition, mPosition + mNormal, up);
        mMatrix     = _proj * view;

        glBindBuffer(GL_UNIFORM_BUFFER, GetUBO());
        auto base = glsl_tool::UBOAddData<decltype(UBOData::mK0)>(0, mK0);
        base = glsl_tool::UBOAddData<decltype(UBOData::mK1)>(base, mK1);
        base = glsl_tool::UBOAddData<decltype(UBOData::mK2)>(base, mK2);
        base = glsl_tool::UBOAddData<decltype(UBOData::mInCone)>(base, mInCone);
        base = glsl_tool::UBOAddData<decltype(UBOData::mOutCone)>(base, mOutCone);
        base = glsl_tool::UBOAddData<decltype(UBOData::mMatrix)>(base, mMatrix);
        base = glsl_tool::UBOAddData<decltype(UBOData::mNormal)>(base, mNormal);
        base = glsl_tool::UBOAddData<decltype(UBOData::mAmbient)>(base, mAmbient);
        base = glsl_tool::UBOAddData<decltype(UBOData::mDiffuse)>(base, mDiffuse);
        base = glsl_tool::UBOAddData<decltype(UBOData::mSpecular)>(base, mSpecular);
        base = glsl_tool::UBOAddData<decltype(UBOData::mPosition)>(base, mPosition);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glViewport(0, 0, viewW, viewH);
        Global::Ref().RefRender().GetMatrixStack().Identity(MatrixStack::kVIEW);
        Global::Ref().RefRender().GetMatrixStack().Identity(MatrixStack::kPROJ);
        Global::Ref().RefRender().GetMatrixStack().Mul(MatrixStack::kVIEW, view);
        Global::Ref().RefRender().GetMatrixStack().Mul(MatrixStack::kPROJ, _proj);
        rt->BindAttachment(RenderTarget::AttachmentType::kDEPTH, RenderTarget::TextureType::k2D, GetSMP());
    }
    else
    {
        Global::Ref().RefRender().GetMatrixStack().Pop(MatrixStack::kPROJ);
        Global::Ref().RefRender().GetMatrixStack().Pop(MatrixStack::kVIEW);
    }
    return count == 0;
}

