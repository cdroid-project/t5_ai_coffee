/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:49:01
 * @FilePath: /t5_ai_demo/src/data/project.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/


#include "project.h"

AppGlobalData  g_appData;

// 判断 现在是不是时间范围内
bool isTimeRange(std::tm star_time, std::tm end_time){
    std::time_t now_tick = std::time(NULL);
    std::tm tm_now = *localtime(&now_tick);

    std::tm tm_star = tm_now;
    std::tm tm_end = tm_now;
    tm_star.tm_hour = star_time.tm_hour; 
    tm_star.tm_min = star_time.tm_min;
    tm_end.tm_hour = end_time.tm_hour; 
    tm_end.tm_min = end_time.tm_min;

    std::time_t star_tick = std::mktime(&tm_star);
    std::time_t end_tick = std::mktime(&tm_end);
    // 开始的时钟大于结束的时钟，比如： 20：00 - 02：00 此时结束时间应该为第二天，需要添加一天
    if(star_time.tm_hour*60+star_time.tm_min > end_time.tm_hour*60+end_time.tm_min){
        end_tick += 86400;
    }
    
    if((star_tick <= now_tick) && (now_tick <= end_tick))
        return true;
    
    return false;
}
