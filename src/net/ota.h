/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:47:21
 * @FilePath: /t5_ai_demo/src/net/ota.h
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#ifndef __ota_h__
#define __ota_h__

#include <comm_class.h>
#include "curldownload.h"

typedef enum{
    OTA_FLAG_DOWMING,
    OTA_FLAG_ERROR,
    OTA_FLAG_UPDATE_ERROR,
    OTA_FLAG_MD5_ERROR,
    OTA_FLAG_SUCCESS,
    OTA_FLAG_MCU_SUCCES,
}OtaUpDateFlag;

void checkUpdata();
void upDateDownLo(int &flag,int &progress);
void runUpdate(int &flag);

class otaUpDateThread :public ThreadTask{
    virtual int  onTask(void *data);
    virtual void onMain(void *data);
};
#endif