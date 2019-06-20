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
}

void Roaming::OnDel()
{
    Global::Ref().RefEvent().Del(_ekid);
    Global::Ref().RefEvent().Del(_emid);
}

void Roaming::OnUpdate(float dt)
{
}

void Roaming::OnEventMouse(const std::any & any)
{
    auto param  = std::any_cast<Window::EventMouseParam>(any);
    auto camera = GetOwner()->GetComponent<Camera>();
    ASSERT_LOG(camera != nullptr, "Not Find Camera");

    auto right  = camera->GetUp()  * camera->GetEye();
    auto offset = right           * param.dy * 0.1f;
    offset     += camera->GetUp() * param.dx * 0.1f;
    auto eye    = camera->GetEye() + offset;

    camera->SetEye()
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
