#pragma once

#ifdef _HIREDIS_BUILD_
	#define HIRFUNC __declspec(dllexport)
#else
	#define HIRFUNC __declspec(dllimport)
#endif
#include <string>

std::string HIRFUNC RedisConnect(const char* ip, const char* auth);

std::string HIRFUNC RedisCommand(const char* cmd);

int HIRFUNC RedisClose();