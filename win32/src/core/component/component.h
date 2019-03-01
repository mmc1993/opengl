#pragma once

#include "../include.h"
#include "../object/object.h"

class Component {
public:
	Component()
		: _owner(nullptr)
		, _active(true) { }
	virtual ~Component() { }
    virtual void OnAdd() = 0;
    virtual void OnDel() = 0;
    virtual void OnUpdate(float dt) = 0;

	bool IsActive() const { return _active; }
	void SetActive(bool active) { _active = active; }
	void SetOwner(Object * owner) { _owner = owner; }
	Object * GetOwner() { return _owner; }

private:
    bool _active;
    Object *_owner;
};