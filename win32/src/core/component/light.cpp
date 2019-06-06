#include "light.h"
#include "transform.h"
#include "../render/render.h"
#include "../cfg/cfg_manager.h"
#include "../res/res_manager.h"
#include "../raw/raw_manager.h"

void Light::NewVolume()
{
    //  考虑把这些动态生成的网格存储在文件里.
    switch (_type)
    {
    case TypeEnum::kDIRECT:
        {
            auto windowW = Global::Ref().RefCfgManager().At("init")->At("window", "w")->ToInt();
            auto windowH = Global::Ref().RefCfgManager().At("init")->At("window", "h")->ToInt();
            auto mesh = std::make_shared<GLMesh>();
            mesh->Init({ { { -1.0f, -1.0f, 0.0f } }, { {  1.0f, -1.0f, 0.0f } },
                            { {  1.0f,  1.0f, 0.0f } }, { { -1.0f,  1.0f, 0.0f } } },
                            { 0, 2, 1, 0, 3, 2 }, GLMesh::Vertex::EnableEnum::kV);
        }
        break;
    case TypeEnum::kPOINT:
        {
            const auto N0 = 32;
            const auto N1 = N0 * 2-2;
            std::vector<uint>           indexs;
            std::vector<GLMesh::Vertex> vertexs;

            vertexs.emplace_back(glm::vec3(0, 1, 0));
            for (auto i = 0; i != N1; ++i)
            {
                indexs.emplace_back(0);
                indexs.emplace_back((i    ) % N1 + 1);
                indexs.emplace_back((i + 1) % N1 + 1);
            }

            auto step = static_cast<float>(M_PI / (N0 - 1));
            for (auto i = 1; i != N0 - 1; ++i)
            {
                auto x = std::sin(step * i);
                auto y = std::cos(step * i);
                auto base = (i - 2) * N1 + 1;
                for (auto j = 0; j != N1; ++j)
                {
                    if (i != 1)
                    {
                        indexs.push_back(base + j);
                        indexs.push_back(base + N1 + j);
                        indexs.push_back(base + N1 + (j + 1) % N1);

                        indexs.push_back(base + j);
                        indexs.push_back(base + N1 + (j + 1) % N1);
                        indexs.push_back(base + (j + 1) % N1);
                    }
                    auto l = x;
                    auto x = std::sin(step * j) * l;
                    auto z = std::cos(step * j) * l;
                    vertexs.emplace_back(glm::vec3(x, y, z));
                }
            }

            auto base = (N0 - 3) * N1 + 1;
            for (auto i = 0; i != N1; ++i)
            {
                indexs.emplace_back(base + i);
                indexs.emplace_back(base + N1);
                indexs.emplace_back(base + (i + 1) % N1);
            }

            vertexs.emplace_back(glm::vec3(0, -1, 0));
        }
        break;
    case TypeEnum::kSPOT:
        {
            const auto N = 32;
            std::vector<uint>           indexs;
            std::vector<GLMesh::Vertex> vertexs;

            vertexs.emplace_back(glm::vec3(0, 0, 0));
            auto step = static_cast<float>(M_PI*2/N);
            for (auto i = 0; i != N; ++i)
            {
                auto x = std::sin(step * i);
                auto y = std::cos(step * i);
                vertexs.emplace_back(glm::vec3(x, y, 1));

                indexs.emplace_back(0);
                indexs.emplace_back( i + 1);
                indexs.emplace_back((i + 1) % N + 1);

                if (i != 0)
                {
                    indexs.emplace_back((i + 1) % N + 1);
                    indexs.emplace_back( i + 1);
                    indexs.emplace_back(1);
                }
            }
        }
        break;
    }
}

Light::Light(TypeEnum type): _type(type), _ubo(0)
{
    //switch (type)
    //{
    //case kDIRECT:
    //    _volume = Global::Ref().RefRawManager().LoadRes<GLMesh>(BUILTIN_MESH_DEFERRED_LIGHT_VOLUME_DIRECT);
    //    break;
    //case kPOINT:
    //    _volume = Global::Ref().RefRawManager().LoadRes<GLMesh>(BUILTIN_MESH_DEFERRED_LIGHT_VOLUME_POINT);
    //    break;
    //case kSPOT:
    //    _volume = Global::Ref().RefRawManager().LoadRes<GLMesh>(BUILTIN_MESH_DEFERRED_LIGHT_VOLUME_SPOT);
    //    break;
    //}
    //_program = Global::Ref().RefRawManager().LoadRes<GLProgram>(BUILTIN_PROGRAM_DEFERRED_LIGHT_VOLUME);
}

//  --------------------------------------------------------------------------------
//  光源实现
void Light::OnUpdate(float dt)
{
    LightCommand command;
    command.mLight    = this;
    command.mMesh     = _volume;
    command.mProgram  = _program;
    command.mTransform= Global::Ref().RefRender().GetMatrixStack().GetM();
    Global::Ref().RefRender().PostCommand(RenderCommand::kLIGHT, command);

    mPosition = command.mTransform * glm::vec4(0, 0, 0, 1);
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

void LightDirect::OnUpdate(float dt)
{
    Light::OnUpdate(dt);

    auto up     = std::abs(mNormal.y) > 0.999f
                ? glm::vec3(0, 0, 1)
                : glm::vec3(0, 1, 0);
    auto right  = glm::cross(up, mNormal);
    up          = glm::cross(mNormal, right);
    _view       = glm::lookAt(mPosition, mPosition + mNormal, up);
    mMatrix     = _proj * _view;

    glBindBuffer(GL_UNIFORM_BUFFER, GetUBO());
    auto base = glsl_tool::UBOAddData<decltype(UBOData::mMatrix)>(0, mMatrix);
    base = glsl_tool::UBOAddData<decltype(UBOData::mNormal)>(base, mNormal);
    base = glsl_tool::UBOAddData<decltype(UBOData::mAmbient)>(base, mAmbient);
    base = glsl_tool::UBOAddData<decltype(UBOData::mDiffuse)>(base, mDiffuse);
    base = glsl_tool::UBOAddData<decltype(UBOData::mSpecular)>(base, mSpecular);
    base = glsl_tool::UBOAddData<decltype(UBOData::mPosition)>(base, mPosition);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void LightDirect::OpenShadow(const glm::vec2 & orthoX, const glm::vec2 & orthoY, const glm::vec2 & orthoZ)
{
    if (GetUBO() == 0)
    {
        glGenBuffers(1, &_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, GetUBO());
        glBufferData(GL_UNIFORM_BUFFER, GetUBOLength(), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    _proj = glm::ortho(orthoX.x, orthoX.y, orthoY.x, orthoY.y, orthoZ.x, orthoZ.y);
}

bool LightDirect::NextDrawShadow(uint count, uint shadow, RenderTarget * rt)
{
    if (0 == count)
    {
        glViewport(0, 0,
            Global::Ref().RefCfgManager().At("init")->At("shadow_map", "w")->ToInt(), 
            Global::Ref().RefCfgManager().At("init")->At("shadow_map", "w")->ToInt());
        Global::Ref().RefRender().GetMatrixStack().Identity(MatrixStack::kVIEW);
        Global::Ref().RefRender().GetMatrixStack().Identity(MatrixStack::kPROJ);
        Global::Ref().RefRender().GetMatrixStack().Mul(MatrixStack::kVIEW, _view);
        Global::Ref().RefRender().GetMatrixStack().Mul(MatrixStack::kPROJ, _proj);
        rt->BindAttachment(RenderTarget::AttachmentType::kDEPTH, RenderTarget::TextureType::k2D, shadow);
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

void LightPoint::OnUpdate(float dt)
{
    Light::OnUpdate(dt);

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

    GetOwner()->GetTransform()->Scale(Light::CalLightDistance(mK0, mK1, mK2, 0.1f));
}

void LightPoint::OpenShadow(const float n, const float f)
{
    auto viewW = Global::Ref().RefCfgManager().At("init")->At("shadow_map", "w")->ToInt();
    auto viewH = Global::Ref().RefCfgManager().At("init")->At("shadow_map", "h")->ToInt();

    if (GetUBO() == 0)
    {
        glGenBuffers(1, &_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, GetUBO());
        glBufferData(GL_UNIFORM_BUFFER, GetUBOLength(), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

	_proj = glm::perspective(glm::radians(90.0f), (float)viewW / (float)viewH, n, f);
    mFar = f; mNear = n;
}

bool LightPoint::NextDrawShadow(uint count, uint shadow, RenderTarget * rt)
{
    if (count != 0)
    {
        Global::Ref().RefRender().GetMatrixStack().Pop(MatrixStack::kPROJ);
        Global::Ref().RefRender().GetMatrixStack().Pop(MatrixStack::kVIEW);
    }
    else
    {
        glViewport(0, 0, 
            Global::Ref().RefCfgManager().At("init")->At("shadow_map", "w")->ToInt(),
            Global::Ref().RefCfgManager().At("init")->At("shadow_map", "w")->ToInt());
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
        rt->BindAttachment(RenderTarget::AttachmentType::kDEPTH, (RenderTarget::TextureType)(RenderTarget::TextureType::k3D + count), shadow);
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

void LightSpot::OnUpdate(float dt)
{
    Light::OnUpdate(dt);

    auto up     = std::abs(mNormal.y) > 0.999f
                ? glm::vec3(0, 0, 1)
                : glm::vec3(0, 1, 0);
    auto right  = glm::cross(up, mNormal);
    up          = glm::cross(mNormal, right);
    _view       = glm::lookAt(mPosition, mPosition + mNormal, up);
    mMatrix     = _proj * _view;

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

    GetOwner()->GetTransform()->Scale(Light::CalLightDistance(mK0, mK1, mK2, 0.1f));
}

void LightSpot::OpenShadow(const float n, const float f)
{
    auto viewW = Global::Ref().RefCfgManager().At("init")->At("shadow_map", "w")->ToInt();
    auto viewH = Global::Ref().RefCfgManager().At("init")->At("shadow_map", "h")->ToInt();

    if (GetUBO() == 0)
    {
        glGenBuffers(1, &_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, GetUBO());
        glBufferData(GL_UNIFORM_BUFFER, GetUBOLength(), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    _proj = glm::perspective(glm::radians(90.0f), (float)viewW / (float)viewH, n, f);
}

bool LightSpot::NextDrawShadow(uint count, uint shadow, RenderTarget * rt)
{
    if (count == 0)
    {
        glViewport(0, 0, 
            Global::Ref().RefCfgManager().At("init")->At("shadow_map", "w")->ToInt(), 
            Global::Ref().RefCfgManager().At("init")->At("shadow_map", "h")->ToInt());
        Global::Ref().RefRender().GetMatrixStack().Identity(MatrixStack::kVIEW);
        Global::Ref().RefRender().GetMatrixStack().Identity(MatrixStack::kPROJ);
        Global::Ref().RefRender().GetMatrixStack().Mul(MatrixStack::kVIEW, _view);
        Global::Ref().RefRender().GetMatrixStack().Mul(MatrixStack::kPROJ, _proj);
        rt->BindAttachment(RenderTarget::AttachmentType::kDEPTH, RenderTarget::TextureType::k2D, shadow);
    }
    else
    {
        Global::Ref().RefRender().GetMatrixStack().Pop(MatrixStack::kPROJ);
        Global::Ref().RefRender().GetMatrixStack().Pop(MatrixStack::kVIEW);
    }
    return count == 0;
}

