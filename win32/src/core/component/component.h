#pragma once

#include "../include.h"

class Object;

class Component {
public:
    Component();
    virtual ~Component();
    virtual void OnAdd() = 0;
    virtual void OnDel() = 0;
    virtual void OnUpdate(float dt) = 0;

    bool IsActive() const;
    void SetActive(bool active);

    void SetOwner(Object * owner);
    Object * GetOwner();

private:
    bool _active;
    Object *_owner;
};