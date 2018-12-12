#pragma once

#include "../include.h"
#include "../object/object.h"

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

    template <class T>
    T * GetParent()
    {
        assert(GetOwner() != nullptr);
        auto parent = GetOwner()->GetParent();
        while (parent != nullptr)
        {
            auto comp = parent->GetComponent<T>();
            if (comp != nullptr)
            {
                return comp;
            }
            parent = parent->GetParent();
        }
        return nullptr;
    }

private:
    bool _active;
    Object *_owner;
};