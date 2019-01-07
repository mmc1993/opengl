#pragma once

#include "../include.h"

class Asset {
public:
	virtual ~Asset()
	{ }

protected:
	Asset()
	{ }

	Asset(Asset &&) = delete;
	Asset(const Asset &) = delete;
	Asset & operator=(const Asset &) = delete;
	Asset & operator=(Asset &&) = delete;
};