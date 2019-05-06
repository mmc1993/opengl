#pragma once

#include "res.h"
#include "../tools/debug_tool.h"

class ResCache {
public:
	bool IsReg(const std::string & url) const
	{
		auto it = _resources.find(url);
		return it != _resources.end();
	}

	bool Reg(const std::string & key, Res * asset)
	{
		return _resources.insert(std::make_pair(key, asset)).second;
	}

	void Del(const std::string & url)
	{
		auto it = _resources.find(url);
		if (it != _resources.end())
		{
			delete it->second;
			_resources.erase(it);
		}
	}

	template <class T>
	T * Get(const std::string & url)
	{
        ASSERT_LOG(IsReg(url), "Get URL: {0}", url);
		auto ret = _resources.at(url);
        ASSERT_LOG(dynamic_cast<T *>(ret), "Get URL: {0}", url);
		return reinterpret_cast<T *>(ret);
	}

	const std::map<std::string, Res *> & Gets()
	{
		return _resources;
	}

private:
	std::map<std::string, Res *> _resources;
};