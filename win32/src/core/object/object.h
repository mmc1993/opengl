#pragma once

#include "../include.h"

class Component;
class Transform;

class Object {
public:
    Object();
    virtual ~Object();
    virtual void OnUpdate(float dt);

    void AddChild(Object * child, uint tag = ~0);
    void DelChild(Object * child);
    void DelChildIdx(uint idx);
    void DelChildTag(uint tag);
	void DelThis();
	Object * GetChildIdx(uint idx);    
	Object * GetChildTag(uint tag);
    std::vector<Object *> & GetChilds();

	void DelComponentAll();
    void AddComponent(Component * component);
    void DelComponent(const std::type_info & type);
    std::vector<Component *> & GetComponents();

    template <class T>
    T * GetComponent()
    {
        auto iter = std::find_if(_components.begin(), _components.end(),
           [](Component * component) { return typeid(*component) == typeid(T); });
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

    Transform * GetTransform();

    void SetCameraFlag(uint flag);
    uint GetCameraFlag() const;

    void SetActive(bool active);
    bool IsActive() const;

    void RootUpdate(float dt);
    void Update(float dt);

    void SetParent(Object * parent);
    Object * GetParent();

protected:
    void DelChild(size_t idx, bool del);
    void DelChild(Object * child, bool del);

private:
    uint _tag;
    bool _active;
	uint _cameraFlag;
    Object * _parent;
	Transform * _transform;
    std::vector<Object *> _childs;
    std::vector<Component *> _components;
};