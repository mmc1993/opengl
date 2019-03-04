#pragma once

#include "../include.h"

namespace string_tool {
	
	inline std::string Join(const std::vector<std::string> & vec, const std::string & str)
	{
		std::string ret;
		auto count = 0;
		if (vec.empty()) { return std::move(ret); }
		for (auto & s : vec) { count += s.size(); }
		
		ret.reserve(count);
		auto it = vec.begin(); ret.append(*it++);
		while (it != vec.end()) { ret.append(*it++); }
		return std::move(ret);
	}

	inline std::vector<std::string> Split(
		const std::string & str, 
		const std::string & sep)
	{
		std::vector<std::string> result;
		auto pos = (size_t)0;
		auto end = str.find(sep, pos);
		while (end != std::string::npos)
		{
			result.push_back(str.substr(pos, end - pos));
			pos = end + sep.size();
			end = str.find(sep, pos);
		}
		if (pos < str.size())
		{
			result.push_back(str.substr(pos, end - pos));
		}
		return std::move(result);
	}

	inline std::string Replace(const std::string & str, 
							   const std::string & sold, 
							   const std::string & snew, 
							   size_t count = (size_t)-1)
	{
		std::vector<std::string> vec;
		auto pos = (size_t)0;
		auto end = str.find(sold, pos);
		while (end != std::string::npos && --count != 0)
		{
			vec.push_back(str.substr(pos, end - pos));
			vec.push_back(snew);
			pos = end + sold.size();
			end = str.find(sold, pos);
		}
		if (pos < str.size())
		{
			vec.push_back(str.substr(pos, end - pos));
		}
		return Join(vec, "");
	}
}