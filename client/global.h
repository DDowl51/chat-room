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
// ��������ʽform�ж��ַ���str�Ƿ���Ϲ淶, �������������Ϣmessage
bool match(std::string& str, std::string form, std::string message);
void input_and_match(std::string& str, std::string form, std::string message);
/* ----- Functions ----- */