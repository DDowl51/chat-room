#pragma once

/* ----- Header files----- */
#include <winsock.h>
#include <iostream>
#include <string>
#include <thread>
#include <mysql.h>
#include <regex>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")
/* ----- Header files----- */


/* ----- Functions ----- */
// 用正则表达式form判断字符串str是否符合规范, 不符合则输出信息message
bool match(std::string& str, std::string form, std::string message);
void input_and_match(std::string& str, std::string form, std::string message);
/* ----- Functions ----- */