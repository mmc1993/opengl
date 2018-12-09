#include "node.h"
#include "../component/component.h"

Node::Node()
    : _tag(std::numeric_limits<size_t>::max())
    , _active(true)
    , _parent(nullptr)
{
}

Node::~Node()
{
}

void Node::Update(float dt)
{
}

void Node::AddChild(Node * child, size_t tag)
{
    child->_tag = tag;
    child->_parent = this;
    _childs.push_back(child);
}

void Node::DelChild(Node * child)
{
    DelChild(child, true);
}

void Node::DelChildIdx(size_t idx)
{
    DelChild(idx, true);
}

void Node::DelChildTag(size_t tag)
{
    auto it = std::find_if(_childs.begin(), _childs.end(), 
        [tag](Node * child) { return child->_tag == tag; });
    if (it != _childs.end())
    {
        DelChild(std::distance(_childs.begin(), it), true);
    }
}

Node * Node::GetChildTag(size_t tag)
{
    auto it = std::find_if(_childs.begin(), _childs.end(),
        [tag](Node * child) { return child->_tag == tag; });
    return it != _childs.end() ? *it : nullptr;
}

Node * Node::GetChildIdx(size_t idx)
{
    assert(idx < _childs.size());
    return *std::next(_childs.begin(), idx);
}

std::vector<Node*>& Node::GetChilds()
{
    return _childs;
}

void Node::AddComponent(Component * component)
{
    _components.push_back(component);
}

void Node::DelComponent(const std::type_info & type)
{
    auto it = std::find_if(_components.begin(), _components.end(),
        [&type](Component * component) { return typeid(*component) == type; });
    if (it != _components.end()) { _components.erase(it); }
}

std::vector<Component*>& Node::GetComponents()
{
    return _components;
}

Component * Node::GetComponent(const std::type_info & type)
{
    auto it = std::find_if(_components.begin(), _components.end(),
        [&type](Component * component) { return typeid(*component) == type; });
    return it != _components.end() ? *it : nullptr;
}

std::vector<Component*> && Node::GetComponentsInChilds(const std::type_info & type)
{
    std::vector<Component*> result{ GetComponent(type) };
    for (auto child : _childs)
    {
        auto ret = std::move(child->GetComponentsInChilds(type));
        result.insert(result.end(), ret.begin(), ret.end());
    }
    return std::move(result);
}

void Node::SetActive(bool active)
{
    _active = active;
}

bool Node::IsActive() const
{
    return _active;
}

void Node::SetParent(Node * parent)
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

Node * Node::GetParent()
{
    return _parent;
}

void Node::DelChild(size_t idx, bool del)
{
    assert(idx < _childs.size());
    auto it = std::next(_childs.begin(), idx);
    (*it)->_parent = nullptr;
    if (del) { delete *it; }
    _childs.erase(it);
}

void Node::DelChild(Node * child, bool del)
{
    auto it = std::find(_childs.begin(), _childs.end(), child);
    assert(it != _childs.end());
    DelChild(std::distance(_childs.begin(), it), del);
}
