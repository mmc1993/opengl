#pragma once

#include "sprite.h"

class SpriteBatch : public Sprite {
public:
	SpriteBatch();
	~SpriteBatch();
	virtual void OnAdd() override;
	virtual void OnDel() override;
	virtual void OnUpdate(float dt) override;
	void SetCount(size_t count) { _count = count; }
private:
	size_t _count;
};