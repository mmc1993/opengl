#pragma once

#include "component.h"

class Camera: public Component {
public:
	enum TypeEnum {
		//	Õý½»
		kORTHOGONAL,
		//	Í¸ÊÓ
		kPERSPECTIVE,
	};

    enum MaskEnum {
        kMASK0 = 0x1,   kMASK1 = 0x2,   kMASK2 = 0x4,   kMASK3 = 0x8,
        kMASK4 = 0x10,  kMASK5 = 0x20,  kMASK6 = 0x30,  kMASK7 = 0x40,
    };

	union Info
	{
		struct Ortho {
			float l, r, t, b;
			float n, f;
		} mOrtho;

		struct Persp {
			float fov;
			float w, h;
			float n, f;
		} mPersp;
	};

public:
    Camera();
    ~Camera();
	void InitPerspective(float fov, float w, float h, float n, float f);
	void InitOrthogonal(float l, float r, float t, float b, float n, float f);
    void LookAt(const glm::vec3 & pos, 
				const glm::vec3 & eye, 
				const glm::vec3 & up);

    void SetMask(const uint mask);
    void SetOrder(const uint order);
	void SetEye(const glm::vec3 & eye);
	void SetPos(const glm::vec3 & pos);
	void SetViewport(const glm::vec4 & viewport);

    uint GetMask() const;
    uint GetOrder() const;
	const glm::vec3 & GetUp() const;
	const glm::vec3 & GetEye() const;
	const glm::vec3 & GetPos() const;
	const glm::mat4 & GetView() const;
	const glm::mat4 & GetProj() const;
	const glm::vec4 & GetViewport() const;

	virtual void OnAdd() override {}
	virtual void OnDel() override {}
    virtual void OnUpdate(float dt) override;

private:
    void Update() const;

private:
	Info _info;
    uint _mask;
    uint _order;
    TypeEnum _type;
    glm::vec4 _viewport;
    glm::vec3 _eye, _pos, _up;

    mutable bool _change;
    mutable glm::mat4 _view;
    mutable glm::mat4 _project;
};