#pragma once


#include <iostream>			/* cout, cin */
#include <string>
#include <thread>
#include <mysql.h>
#include <mutex>
#include <unordered_map>
#include <tuple>
#include <set>
#include <winsock.h>		/* 网络通信 */
#include <random>			/* srand(), rand() */
#include <stdio.h>			/* printf */
#include <time.h>			/* time_t, struct tm, time, localtime, asctime */
#include <format>			/* std::format */

#include "HiRedis.h";

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "HiRedis.lib")
#define SERVER_PORT 8023

using connect_info = std::tuple<bool, std::string, std::string, int, int>;


enum LogLevel {
	info = 0,
	warning,
	error
};

std::string addPrefixZero(int time);
std::string formatTime();


template<typename... Args>
std::string output(int loglevel, std::string rt_fmt, Args&&... args) {
    std::string fmt;
    switch (loglevel) {
    case LogLevel::warning:
        fmt = "\033[33m"; // 黄色
        break;
    case LogLevel::error:
        fmt = "\033[31m"; // 红色
        break;
    case LogLevel::info:
    default:
        fmt = "\033[0m";  // 默认颜色
        break;
    }
    fmt = fmt + "[{}]" + rt_fmt + "\033[0m";    // (ANSI escape code)[Time]Message...(ANSI escape code)
    return std::vformat(fmt, std::make_format_args(formatTime(), args...));
}

template<typename... Args>
int log(int loglevel, std::string fmt, Args&&... args) {
    std::cout << output(loglevel, fmt, args...) << std::endl;
    return 0;
}
