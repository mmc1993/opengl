#include "component.h"

Component::Component()
    : _active(true)
{
}

Component::~Component()
{
}

bool Component::IsActive() const
{
    return _active;
}

void Component::SetActive(bool active)
{
    _active = active;
}

void Component::SetOwner(Node * owner)
{
    _owner = owner;
}

Node * Component::GetOwner()
{
    return _owner;
}
