#include "object.h"
#include "../mmc.h"
#include "../asset/shader.h"
#include "../render/render.h"
#include "../component/component.h"
#include "../component/transform.h"

Object::Object()
    : _tag(std::numeric_limits<size_t>::max())
    , _active(true)
    , _parent(nullptr)
{
    _transform = new Transform();
    AddComponent(_transform);
}

Object::~Object()
{
}

void Object::OnUpdate(float dt)
{
	//	TODO MMC
    Render::Command command;
    command.mType = Render::CommandType::kTRANSFORM;
    command.mCameraID = GetCameraID();
    command.mCallFn = []() {
        float vertices[] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.0f,  0.5f, 0.0f
        };

        //Shader shader;
        //auto ret = shader.InitFromFile("res/shader/1.vsh", "res/shader/1.fsh");
        //shader.Bind();

        //unsigned int vao;
        //glGenVertexArrays(1, &vao);
        //auto r = glGetError();
        //glBindVertexArray(vao);
        //r = glGetError();

        //GLuint vbo;
        //glGenBuffers(1, &vbo);
        //r = glGetError();
        //glBindBuffer(GL_ARRAY_BUFFER, vbo);
        //r = glGetError();
        //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        //r = glGetError();

        //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        //r = glGetError();
        //glEnableVertexAttribArray(0);
        //r = glGetError();

        //glDrawArrays(GL_TRIANGLES, 0, 3);
        //r = glGetError();
    };
    mmc::mRender.PostCommand(command);
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

Transform * Object::GetTransform()
{
    return _transform;
}

size_t Object::GetCameraID() const
{
    return _cameraID;
}

void Object::SetCameraID(size_t id)
{
    _cameraID = id;
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

		//	TODO MMC
        PushTransform();
        for (auto child : _childs)
        {
            child->Update(dt);
        }
        PopTransform();
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

void Object::PushTransform()
{
    Render::Command cmdBefore;
    cmdBefore.mCameraID = _cameraID;
    cmdBefore.mType = Render::CommandType::kTRANSFORM;
    cmdBefore.mCallFn = Render::CommandTransform(true, &GetTransform()->GetMatrix());
    mmc::mRender.PostCommand(cmdBefore);
}

void Object::PopTransform()
{
    Render::Command cmdAfter;
    cmdAfter.mCameraID = _cameraID;
    cmdAfter.mType = Render::CommandType::kTRANSFORM;
    cmdAfter.mCallFn = Render::CommandTransform(false, nullptr);
    mmc::mRender.PostCommand(cmdAfter);
}