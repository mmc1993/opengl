#pragma once

#include "component.h"

class Roaming : public Component {
public:
    enum DirectEnum
    {
        kNONE   = 0,
        kFRONT  = 0x1,
        kBACK   = 0x2,
        kLEFT   = 0x4,
        kRIGHT  = 0x8,
        kUP     = 0x10,
        kDOWN   = 0x20,
    };

public:
    Roaming();
    ~Roaming();

    virtual void OnAdd() override;
    virtual void OnDel() override;
    virtual void OnUpdate(float dt) override;

private:
    void OnEventMouse(const std::any & any);
    void OnEventKeybord(const std::any & any);

private:
    size_t _ekid;
    size_t _emid;
    uint _direct;
    //glm::vec
};