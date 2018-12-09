#pragma once

#include "../include.h"

class Component;

class Node {
public:
    Node();
    virtual ~Node();

    virtual void OnUpdate(float dt);

    void AddChild(Node * child, size_t tag = std::numeric_limits<size_t>::max());
    void DelChild(Node * child);
    void DelChildIdx(size_t idx);
    void DelChildTag(size_t tag);
    Node * GetChildTag(size_t tag);
    Node * GetChildIdx(size_t idx);
    std::vector<Node *> & GetChilds();

    void AddComponent(Component * component);
    void DelComponent(const std::type_info & type);
    std::vector<Component *> & GetComponents();
    Component * GetComponent(const std::type_info & type);
    std::vector<Component *> && GetComponentsInChilds(const std::type_info & type);
    
    void SetActive(bool active);
    bool IsActive() const;
    void Update(float dt);

    void SetParent(Node * parent);
    Node * GetParent();

protected:
    void DelChild(size_t idx, bool del);
    void DelChild(Node * child, bool del);

private:
    size_t _tag;
    bool _active;
    Node*_parent;
    std::vector<Node *> _childs;
    std::vector<Component *> _components;
};