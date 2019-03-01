#pragma once

#include "asset.h"

class AssetCache {
public:
	bool IsReg(const std::string & url) const
	{
		auto it = _assets.find(url);
		return it != _assets.end();
	}

	bool Reg(const std::string & key, Asset * asset)
	{
		return _assets.insert(std::make_pair(key, asset)).second;
	}

	void Del(const std::string & url)
	{
		auto it = _assets.find(url);
		if (it != _assets.end())
		{
			delete it->second;
			_assets.erase(it);
		}
	}

	template <class T>
	T * Get(const std::string & url)
	{
		assert(IsReg(url));
		auto ret = _assets.at(url);
		assert(dynamic_cast<T *>(ret));
		return reinterpret_cast<T *>(ret);
	}

	const std::map<std::string, Asset *> & Gets()
	{
		return _assets;
	}

private:
	std::map<std::string, Asset *> _assets;
};