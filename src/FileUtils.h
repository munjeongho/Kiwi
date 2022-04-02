#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <kiwi/Types.h>
#include "StrUtils.h"

namespace kiwi
{
#if defined(__GNUC__) && __GNUC__ < 5
#define openFile(...) std::ifstream(__VA_ARGS__)
#else
	inline std::ifstream openFile(const std::string& filePath, std::ios_base::open_mode mode = std::ios_base::in)
	{
		std::ifstream f;
		auto exc = f.exceptions();
		f.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
#if defined(_WIN32) || defined(_WIN64)
			f.open((const wchar_t*)utf8To16(filePath).c_str(), mode);
#else
			f.open(filePath, mode);
#endif
		}
		catch (std::ios_base::failure& e)
		{
			throw Exception{ "Cannot open file : " + filePath };
		}
		f.exceptions(exc);
		return f;
	}
#endif
}