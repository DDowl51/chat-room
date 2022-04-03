#include "global.h"


/*
struct tm
{
    int tm_sec;   // seconds after the minute - [0, 60] including leap second
    int tm_min;   // minutes after the hour - [0, 59]
    int tm_hour;  // hours since midnight - [0, 23]
    int tm_mday;  // day of the month - [1, 31]
    int tm_mon;   // months since January - [0, 11]
    int tm_year;  // years since 1900
    int tm_wday;  // days since Sunday - [0, 6]
    int tm_yday;  // days since January 1 - [0, 365]
    int tm_isdst; // daylight savings time flag
}; 
 */

std::string addPrefixZero(int time) {
    std::string ret = std::to_string(time);
    if (ret.length() == 1)
        ret = "0" + ret;
    return ret;
}

std::string formatTime() {
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    return (addPrefixZero(timeinfo->tm_hour) + ":" + addPrefixZero(timeinfo->tm_min) + ":" + addPrefixZero(timeinfo->tm_sec));
}

