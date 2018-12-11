#include "object.h"
#include "../component/component.h"

Object::Object()
    : _tag(std::numeric_limits<size_t>::max())
    , _active(true)
    , _parent(nullptr)
{
}

Object::~Object()
{
}

void Object::OnUpdate(float dt)
{
}

void Object::AddChild(Object * child, size_t tag)
{
    child->_tag = tag;
    child->_parent = this;
    _childs.push_back(child);
}

void Object::DelChild(Object * child)
{
    DelChild(child, true);
}

void Object::DelChildIdx(size_t idx)
{
    DelChild(idx, true);
}

void Object::DelChildTag(size_t tag)
{
    auto it = std::find_if(_childs.begin(), _childs.end(), 
        [tag](Object * child) { return child->_tag == tag; });
    if (it != _childs.end())
    {
        DelChild(std::distance(_childs.begin(), it), true);
    }
}

Object * Object::GetChildTag(size_t tag)
{
    auto it = std::find_if(_childs.begin(), _childs.end(),
        [tag](Object * child) { return child->_tag == tag; });
    return it != _childs.end() ? *it : nullptr;
}

Object * Object::GetChildIdx(size_t idx)
{
    assert(idx < _childs.size());
    return *std::next(_childs.begin(), idx);
}

std::vector<Object*>& Object::GetChilds()
{
    return _childs;
}

void Object::AddComponent(Component * component)
{
    _components.push_back(component);
    component->SetOwner(this);
    component->OnAdd();
}

void Object::DelComponent(const std::type_info & type)
{
    auto it = std::find_if(_components.begin(), _components.end(),
        [&type](Component * component) { return typeid(*component) == type; });
    if (it != _components.end()) 
    {
        _components.erase(it);
        (*it)->OnDel();
        delete *it;
    }
}

std::vector<Component*>& Object::GetComponents()
{
    return _components;
}

void Object::SetActive(bool active)
{
    _active = active;
}

bool Object::IsActive() const
{
    return _active;
}

void Object::Update(float dt)
{
    if (IsActive())
    {
        OnUpdate(dt);

        for (auto component : _components)
        {
            if (component->IsActive())
            {
                component->OnUpdate(dt);
            }
        }

        for (auto child : _childs)
        {
            child->Update(dt);
        }
    }
}

void Object::SetParent(Object * parent)
{
    if (nullptr != _parent)
    {
        _parent->DelChild(this, false);
    }
    if (nullptr != parent)
    {
        parent->AddChild(this, _tag);
    }
}

Object * Object::GetParent()
{
    return _parent;
}

void Object::DelChild(size_t idx, bool del)
{
    assert(idx < _childs.size());
    auto it = std::next(_childs.begin(), idx);
    (*it)->_parent = nullptr;
    if (del) { delete *it; }
    _childs.erase(it);
}

void Object::DelChild(Object * child, bool del)
{
    auto it = std::find(_childs.begin(), _childs.end(), child);
    assert(it != _childs.end());
    DelChild(std::distance(_childs.begin(), it), del);
}
