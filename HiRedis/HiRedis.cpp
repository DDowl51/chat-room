#include <hiredis.h>
#include <Win32_Interop/win32fixes.h>

#include "HiRedis.h"

#pragma comment(lib, "hiredis.lib")
#pragma comment(lib, "Win32_Interop.lib")

redisContext* server;
redisReply* re;
std::string HIRFUNC RedisConnect(const char* ip, const char* auth) {
	server = redisConnect(ip, 6379);
	if (server->err)
		return std::string(server->errstr);
	re = (redisReply*)redisCommand(server, auth);
	return std::string(re->str);
}

std::string HIRFUNC RedisCommand(const char* cmd)
{
	re = (redisReply*)redisCommand(server, cmd);
	// printf("executing %s\n", cmd);
	// memcpy(ret, re->str, strlen(re->str));
	if (re->str) {
		// printf("%s\n", re->str);
		return std::string(re->str);
	}
	return "(null)";
	// ret = re->str;
}

int HIRFUNC RedisClose() {
	freeReplyObject(re);
	redisFree(server);
	return 0;
}