#pragma once


#include "../include.h"

class Light {
public:
	enum LightType {
		kAMBIENT,
		kPOINT,
		kSPOT,
		MAX,
	};

public:
	virtual ~Light()
	{ }

	LightType GetType() const
	{
		return _type;
	}

protected:
	Light(LightType type): _type(type)
	{ }

private:
	LightType _type;
};

class LightAmbient : public Light {
public:
	LightAmbient(): Light(Light::kAMBIENT)
	{ }

	~LightAmbient()
	{ }

	float GetValue() const
	{
		return _value;
	}

	void SetValue(float value)
	{
		_value = value;
	}

public:
	float _value;
};

class LightPoint : public Light {
public:
	struct Value {
		glm::vec3 pos;
		glm::vec3 color;
		float min;
		float max;
	};

public:
	LightPoint(): Light(Light::kPOINT)
	{ }

	~LightPoint()
	{ }

	const Value & GetValue() const
	{
		return _value;
	}

	void SetValue(const Value & value)
	{
		_value = value;
	}

private:
	Value _value;
};

class LightSpot : public Light {
public:
	struct Value {
		glm::vec3 pos;
		glm::vec3 dir;
		glm::vec3 color;
		float min;
		float max;
		float mincone;
		float maxcone;
	};

public:
	LightSpot(): Light(Light::kSPOT)
	{ }

	~LightSpot()
	{ }

	const Value & GetValue() const
	{
		return _value;
	}

	void SetValue(const Value & value)
	{
		_value = value;
	}

private:
	Value _value;
};