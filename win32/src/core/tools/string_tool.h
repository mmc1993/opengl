#pragma once

#include "../include.h"

namespace string_tool {
	inline std::vector<std::string_view> && Split(
		const std::string_view & str, 
		const std::string_view & sep)
	{
		std::vector<std::string_view> result;
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
}