#pragma once

#include "../include.h"

class Window;
class Component;
class Transform;

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

    template <class T>
    T * GetComponent()
    {
        auto iter = std::find_if(_components.begin(), _components.end(),
            [](Component * component) 
            { 
                return typeid(*component) == typeid(T); 
            });
        return reinterpret_cast<T *>(iter != _components.end() ? *iter : nullptr);
    }

    template <class T>
    std::vector<T *> GetComponentsInChilds()
    {
        std::vector<T *> result{};
        auto self = GetComponent<T>();
        if (self != nullptr)
        {
            result.push_back(self);
        }
        for (auto child : _childs)
        {
            auto ret = std::move(child->GetComponentsInChilds<T>());
            result.insert(result.end(), ret.begin(), ret.end());
        }
        return std::move(result);
    }

    void Update(float dt);

    Transform * GetTransform();
    size_t GetCameraID() const;
    void SetCameraID(size_t id);
    void SetActive(bool active);
    bool IsActive() const;

    void SetParent(Object * parent);
    Object * GetParent();

protected:
    void DelChild(size_t idx, bool del);
    void DelChild(Object * child, bool del);

private:
    size_t _tag;
    bool _active;
    size_t _cameraID;
    Object * _parent;
    Transform * _transform;
    std::vector<Object *> _childs;
    std::vector<Component *> _components;
};