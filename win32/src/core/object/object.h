#pragma once

#include "../include.h"

class Component;

class Object {
public:
    Object();
    virtual ~Object();

    virtual void OnUpdate(float dt);

    void AddChild(Object * child, size_t tag = std::numeric_limits<size_t>::max());
    void DelChild(Object * child);
    void DelChildIdx(size_t idx);
    void DelChildTag(size_t tag);
    Object * GetChildTag(size_t tag);
    Object * GetChildIdx(size_t idx);
    std::vector<Object *> & GetChilds();

    void AddComponent(Component * component);
    void DelComponent(const std::type_info & type);
    std::vector<Component *> & GetComponents();
    Component * GetComponent(const std::type_info & type);
    std::vector<Component *> && GetComponentsInChilds(const std::type_info & type);
    
    void SetActive(bool active);
    bool IsActive() const;
    void Update(float dt);

    void SetParent(Object * parent);
    Object * GetParent();

protected:
    void DelChild(size_t idx, bool del);
    void DelChild(Object * child, bool del);

private:
    size_t _tag;
    bool _active;
    Object * _parent;
    std::vector<Object *> _childs;
    std::vector<Component *> _components;
};