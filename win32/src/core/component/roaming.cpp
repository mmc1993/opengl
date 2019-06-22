#include "roaming.h"
#include "camera.h"
#include "transform.h"
#include "../window/window.h"
#include "../event/event.h"
#include "../event/event_enum.h"

Roaming::Roaming()
{
}

Roaming::~Roaming()
{
}

void Roaming::OnAdd()
{
    _ekid = Global::Ref().RefEvent().Add(EventTypeEnum::kWINDOW_KEYBOARD,   std::bind(&Roaming::OnEventKeybord, this, std::placeholders::_1));
    _emid = Global::Ref().RefEvent().Add(EventTypeEnum::kWINDOW_MOUSE_MOVEED, std::bind(&Roaming::OnEventMouse, this, std::placeholders::_1));
    _emid = Global::Ref().RefEvent().Add(EventTypeEnum::kWINDOW_MOUSE_BUTTON, std::bind(&Roaming::OnEventMouse, this, std::placeholders::_1));
}

void Roaming::OnDel()
{
    Global::Ref().RefEvent().Del(_ekid);
    Global::Ref().RefEvent().Del(_emid);
}

void Roaming::OnUpdate(float dt)
{
    auto camera = GetOwner()->GetComponent<Camera>();
    ASSERT_LOG(camera != nullptr, "Not Find Camera");
    auto up     = camera->GetUp();
    auto eye    = camera->GetEye();
    auto pos    = camera->GetPos();
    auto right  = glm::cross(up, eye);
    //  计算eye
    if (_mdiff.z != 0)
    {
        eye += _mdiff.y * up;
        eye += _mdiff.x * right;
        eye.y = eye.y > 0.999f ? 0.999f
            : eye.y < -0.999f ? -0.999f
            : eye.y;
    }
    //  计算pos
    if ((_direct & kUP)     != 0) { pos.y += 1 * 0.1f; }
    if ((_direct & kDOWN)   != 0) { pos.y -= 1 * 0.1f; }
    if ((_direct & kFRONT)  != 0) { pos   += eye * 0.1f; }
    if ((_direct & kBACK)   != 0) { pos   -= eye * 0.1f; }
    if ((_direct & kLEFT)   != 0) { pos   += glm::cross(up, eye) * 0.1f; }
    if ((_direct & kRIGHT)  != 0) { pos   -= glm::cross(up, eye) * 0.1f; }

    _mdiff.x = 0;
    _mdiff.y = 0;
    //  调整相机
    camera->LookAt(pos, pos + eye, up);
}

void Roaming::OnEventMouse(const std::any & any)
{
    const auto windowW = Global::Ref().RefWindow().GetW();
    const auto windowH = Global::Ref().RefWindow().GetH();
    const auto margin = (float)std::sqrt(windowW * windowH) * 0.1f;

    auto param  = std::any_cast<Window::EventMouseParam>(any);
    _mdiff.z = param.btn == 0 && param.act == 1 ? 1.0f : 0.0f;
    _mdiff.x = param.dx * 0.01f;
    _mdiff.y = param.dy * 0.01f;
}

void Roaming::OnEventKeybord(const std::any & any)
{
    auto & param = std::any_cast<const Window::EventKeyParam &>(any);
    _direct = param.act == 1 && param.key == 'W' ? _direct | kFRONT :
        param.act == 0 && param.key == 'W' ? _direct ^ kFRONT : _direct;

    _direct = param.act == 1 && param.key == 'S' ? _direct | kBACK :
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
