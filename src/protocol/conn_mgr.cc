/*
 * @Author: AZhang
 * @Email: azhangxie0612@gmail.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:44:19
 * @FilePath: /t5_ai_demo/src/protocol/conn_mgr.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by AZhang, All Rights Reserved. 
 * 
**/

#include "conn_mgr.h"

#include <core/app.h>
#include <fstream>

#include "conf_mgr.h"
#include "data.h"
#include "wind_mgr.h"
#include "btn_mgr.h"
#include "tcp_conn_mgr.h"
#define TICK_TIME 50 // tick触发时间（毫秒）

//////////////////////////////////////////////////////////////////

CConnMgr::CConnMgr() {
    mPacket            = new SDHWPacketBuffer(BT_MCU);
    mUartMCU           = 0;
    mNextEventTime     = 0;
    mLastSendTime      = 0;
    mLastStatusTime    = 0;
    mSetData           = 0;
    mMcuType           = 0;
    mLastWarnTime      = 0;
    mLastBeepTime      = 0;
    commErrorCount     = 0;
    mSendOTACount      = 0;
    mSendOTALen        = 0;
    mIsFactory         = false;
    mSendOTAData       = nullptr;

    g_objHandler->addHandler(BT_MCU, this);
}

CConnMgr::~CConnMgr() {
    __del(mUartMCU);
    App::getInstance().removeEventHandler(this);
}

int CConnMgr::init() {
    UartOpenReq ss;

    snprintf(ss.serialPort, sizeof(ss.serialPort), "/dev/ttyS3");
    ss.speed     = 115200;
    ss.flow_ctrl = 0;
    ss.databits  = 8;
    ss.stopbits  = 1;
    ss.parity    = 'N';

    mLastWarnTime = SystemClock::uptimeMillis();
    mUartMCU = new UartClient(mPacket, BT_MCU, ss, "10.0.0.55", 1616, 0);
    mUartMCU->init();

    // 启动延迟一会后开始发包
    mNextEventTime = SystemClock::uptimeMillis() + TICK_TIME * 10;
    App::getInstance().addEventHandler(this);

    return 0;
}

int CConnMgr::checkEvents() {
    int64_t curr_tick = SystemClock::uptimeMillis();
    if (curr_tick >= mNextEventTime) {
        mNextEventTime = curr_tick + TICK_TIME;
        return 1;
    }
    return 0;
}

int CConnMgr::handleEvents() {
    int64_t now_tick = SystemClock::uptimeMillis();

    if (mUartMCU) mUartMCU->onTick();

    if ( mIsFactory && (now_tick - mLastSendTime >= 1000) ) { 
        setBuzzer_SetBit(1,1,1);
    }
    // else if(now_tick - mLastSendTime >= 100){ send2MCU(MODBUS_CMD_READ_ALL); }

    return 1;
}

void CConnMgr::setFactoryCallback(UartClient::uartCallback cb){
    // if(cb){
    //     mIsFactory = true;
    //     mUartMCU->setProductCallback(cb);
    // }else{
    //     mIsFactory = false;
    //     mUartMCU->setProductCallback(nullptr);
    // }
}

// 关机指令
void CConnMgr::setShutDown(bool isShutDown){
    mSetData = 0;
    mSndData[mSetData++] = 0x02;
    mSndData[mSetData++] = CMD_SHUTDOWN;
    mSndData[mSetData++] = isShutDown?0x00:0x55;
    send2MCU();
}

// 预热指令
void CConnMgr::setPreheating(uint8_t coffeeTemp, uint8_t steamTemp){
    mSetData = 0;
    mSndData[mSetData++] = 0x03;
    mSndData[mSetData++] = CMD_PRE_HEATING;
    mSndData[mSetData++] = coffeeTemp;
    mSndData[mSetData++] = steamTemp;
    send2MCU();
}

// 磨豆指令
void CConnMgr::setGrindBean(bool flag, uint8_t powder, uint8_t mode){
    mSetData = 0;
    mSndData[mSetData++] = 0x04;
    mSndData[mSetData++] = CMD_GRIND_BEAN;
    mSndData[mSetData++] = flag;
    mSndData[mSetData++] = powder;
    mSndData[mSetData++] = mode;
    send2MCU();
}

// 萃取指令
void CConnMgr::setExtraction(bool flag, uint8_t type, uint8_t soakTime, uint8_t soakTemp, uint8_t extWater, uint8_t extTemp,uint8_t amrHotWater ,uint8_t amrHotWaterTemp){
    mSetData = 0;
    mSndData[mSetData++] = 0x09;
    mSndData[mSetData++] = CMD_EXTRACTION;
    mSndData[mSetData++] = flag;
    mSndData[mSetData++] = type;
    mSndData[mSetData++] = soakTime;
    mSndData[mSetData++] = soakTemp;
    mSndData[mSetData++] = extWater;
    mSndData[mSetData++] = extTemp;
    mSndData[mSetData++] = amrHotWater;
    mSndData[mSetData++] = amrHotWaterTemp;
    send2MCU();
}

// 电子秤校准
void CConnMgr::setElectronicScaleCal(bool flag){
    mSetData = 0;
    mSndData[mSetData++] = 0x02;
    mSndData[mSetData++] = CMD_ELECTRONIC_SCALE_CAL;
    mSndData[mSetData++] = flag;
    send2MCU();
}

// 电子秤
void CConnMgr::setElectronicScale(uint8_t type,uint8_t weight){
    mSetData = 0;
    mSndData[mSetData++] = 0x03;
    mSndData[mSetData++] = CMD_ELECTRONIC_SCALE;
    mSndData[mSetData++] = type;
    mSndData[mSetData++] = weight;
    send2MCU();
}

// 蜂鸣器、SetBit       0x25(37)
void CConnMgr::setBuzzer_SetBit(uint8_t num,uint8_t beepTime,uint8_t beepStopTime){
    mSetData = 0;
    mSndData[mSetData++] = 0x04;
    mSndData[mSetData++] = CMD_BEEP;
    mSndData[mSetData++] = num;
    mSndData[mSetData++] = beepTime;
    mSndData[mSetData++] = beepStopTime;
    send2MCU();
}
// BootLoader 升级信息下发
void CConnMgr::setBootLoaderInfo(){
    mSetData = 0;
    mSndData[mSetData++] = 0x04;
    mSndData[mSetData++] = CMD_UPDATE_INFO;
    mSndData[mSetData++] = (mAllOTALen >> 8) & 0xff;
    mSndData[mSetData++] = mAllOTALen & 0xff;
    mSndData[mSetData++] = mOTADataSum;
    send2MCU();
}
// 固件下发指令
void CConnMgr::sendBootLoaderData(){
    int sendLen = mAllOTALen-mSendOTALen<240?mAllOTALen-mSendOTALen:240;
    mSetData = 0;
    mSndData[mSetData++] = sendLen+3;
    mSndData[mSetData++] = CMD_UPDATE_DATA;
    mSndData[mSetData++] = mSendOTACount++;
    mSndData[mSetData++] = sendLen;
    memcpy(mSndData+mSetData,mSendOTAData+mSendOTALen,sendLen);
    mSetData+=sendLen;
    mSendOTALen += sendLen;
    send2MCU();
}
// 固件下载校验结果查询指令
void CConnMgr::sendBootLoaderCheck(){
    mSetData = 0;
    mSndData[mSetData++] = 0x01;
    mSndData[mSetData++] = CMD_UPDATE_CHECK_END;
    send2MCU();
}

// 设备自检
void CConnMgr::setDeviceSelfTest(){
    mSetData = 0;
    mSndData[mSetData++] = 0x01;
    mSndData[mSetData++] = CMD_DEVICE_SELF_TEST;
    send2MCU();
}
// 自清洁（除垢）
void CConnMgr::setCleanSelf(bool flag, uint8_t type){
    mSetData = 0;
    mSndData[mSetData++] = 0x03;
    mSndData[mSetData++] = CMD_CLEAN_SELF;
    mSndData[mSetData++] = flag;
    mSndData[mSetData++] = type;
    send2MCU();
}

// 大师浓缩
void CConnMgr::setMasEsp(bool flag, uint8_t extTemp, uint8_t soakTime,std::vector<MasEspStepDataStr> stepDataList){
    mSetData = 0;
    mSndData[mSetData++] = 0x0E;
    mSndData[mSetData++] = CMD_MAS_ESP;
    mSndData[mSetData++] = flag;
    mSndData[mSetData++] = extTemp;
    mSndData[mSetData++] = soakTime;
    for(int i=0;i<5;i++){
        if(i < stepDataList.size()){
            mSndData[mSetData++] = stepDataList.at(i).pressure;
            mSndData[mSetData++] = stepDataList.at(i).water;
        }else{
            mSndData[mSetData++] = 0x00;
            mSndData[mSetData++] = 0x00;
        }
    }
    send2MCU();
}

// 手冲咖啡
void CConnMgr::setHandWash(bool flag, uint8_t extTemp,std::vector<FormStepDataStr> stepDataList){
    mSetData = 0;
    mSndData[mSetData++] = 0x0F;
    mSndData[mSetData++] = CMD_HAND_WASH;
    mSndData[mSetData++] = flag;
    mSndData[mSetData++] = extTemp;
    for(int i=0;i<4;i++){
        if(i < stepDataList.size()){
            mSndData[mSetData++] = stepDataList.at(i).water;
            mSndData[mSetData++] = stepDataList.at(i).flowRate;
            mSndData[mSetData++] = stepDataList.at(i).interTime;
        }else{
            mSndData[mSetData++] = 0x00;
            mSndData[mSetData++] = 0x00;
            mSndData[mSetData++] = 0x00;
        }
    }
    send2MCU();
}

// 萃茶
void CConnMgr::setExtTea(bool flag, uint8_t extTemp, bool isRebrew,ExtractTeaSndModeDataStr stepData){
    mSetData = 0;
    mSndData[mSetData++] = 0x0D;
    mSndData[mSetData++] = CMD_EXTRACTION_TEA;
    mSndData[mSetData++] = flag;
    mSndData[mSetData++] = extTemp;
    mSndData[mSetData++] = isRebrew;
   
    // 阶段一 洗茶
    mSndData[mSetData++] = isRebrew?0:stepData.washTeaWater;
    mSndData[mSetData++] = isRebrew?0:stepData.washTeaFlowRate;
    mSndData[mSetData++] = isRebrew?0:stepData.washTeaInterTime;
    
    // 阶段二 泡茶
    mSndData[mSetData++] = isRebrew?0:stepData.soakTeaWater;
    mSndData[mSetData++] = isRebrew?0:stepData.soakTeaFlowRate;
    mSndData[mSetData++] = isRebrew?0:stepData.soakTeaInterTime;

    // 阶段三 冲茶
    mSndData[mSetData++] = stepData.makeTeaWater;
    mSndData[mSetData++] = stepData.makeTeaFlowRate;
    mSndData[mSetData++] = stepData.makeTeaInterTime;

    send2MCU();
}

// 异常上报指令的回应帧
void CConnMgr::reportErrorDataFrame(){
    mSetData = 0;
    mSndData[mSetData++] = 0x02;
    mSndData[mSetData++] = CMD_ERROR;
    mSndData[mSetData++] = 0x5A;
    send2MCU();
}

// 机器设置指令
void CConnMgr::setDeviceData(){
    mSetData = 0;
    mSndData[mSetData++] = 0x06;
    mSndData[mSetData++] = CMD_DEVICE_SET;
    mSndData[mSetData++] = g_appData.beanGrindMode & 0xff;
    mSndData[mSetData++] = g_appData.isMute;
    mSndData[mSetData++] = 0x00;
    mSndData[mSetData++] = 0x00;
    mSndData[mSetData++] = 0x00;
    send2MCU();

}

// 设置机器重置状态（停掉所有的负载，如在ota中、恢复出厂设置中）
void CConnMgr::setResetStatus(bool flag){
    if(flag){

    }
    
    mSetData++;
}

void CConnMgr::send2MCU() {
    return;
    BuffData *bd = mPacket->obtain();
    UI2MCU   snd(bd);

    for (int i = 0; i < mSetData; i++) { snd.setData(i+BUF_SND_D1, mSndData[i]);}

    if(snd.getCMD() == CMD_ERROR){
        snd.setData(0,0x5B);
    }
    snd.setLen(mSetData+4);
    snd.checkcode();    // 修改检验位
    // LOGE("send Beep = %d",statusByte & 0x01);
    LOG(INFO) << "send to mcu. bytes=" << hexstr(bd->buf, bd->len);
    mUartMCU->send(bd);
    
    mSetData      = 0;
    mLastSendTime = SystemClock::uptimeMillis();
}

bool CConnMgr::getVersion(int &a, int &b, int &c) {
    if (mMcuType == 0) return false;
    a = mMcuType;
    return true;
}

void CConnMgr::onCommDeal(IAck *ack) {
    LOG(INFO) << "hex str=" << hexstr(ack->mBuf, ack->mDlen);
    if(mIsFactory) return;
    int64_t now_tick = SystemClock::uptimeMillis();
    
    switch(ack->getData(BUF_RCV_D2)){
        case CMD_SHUTDOWN:{
            if(ack->getData(BUF_RCV_D3) == 0x5A){
                if(g_appData.isShutDown)g_objBtnMgr->shutDown();
            }
            break;
        }case CMD_PRE_HEATING:{
            if(!ack->getData(BUF_RCV_D3)){
                g_appData.coffeePreheatPtc = 100;
            }else{
                int oreheatPtc = ack->getData(BUF_RCV_D4);
                oreheatPtc = std::min(oreheatPtc,ack->getData(BUF_RCV_D5));

                g_appData.coffeePreheatPtc = oreheatPtc>100?100:oreheatPtc;
            }
            if(g_appData.machineState == MC_STATE_NONE || g_appData.machineState == MC_STATE_POWDER){
                g_appData.statusChangeFlag |= CS_PRE_HEATING;
            }
            break;
        }case CMD_GRIND_BEAN:{
            bool isGrindBeaning = g_appData.machineState & MC_STATE_POWDER;
            if(isGrindBeaning && (g_appData.grindBean != ack->getData(BUF_RCV_D3))){
                g_appData.grindBean = ack->getData(BUF_RCV_D3);
                g_appData.statusChangeFlag |= CS_GRIND_BEAN_DATA_CHANGE;
            }
            break;
        }case CMD_EXTRACTION:{
            if(ack->getData(BUF_RCV_D4) == EXT_MODE_HOT_WATER){
                bool isExtracting = g_appData.machineState & MC_STATE_HOT_WATER;
                if(isExtracting && !ack->getData(BUF_RCV_D3)){
                    g_appData.machineState &= ~MC_STATE_HOT_WATER;
                    g_appData.statusChangeFlag |= CS_HOT_WATER_DOWN;
                }
            }else if(ack->getData(BUF_RCV_D4) == EXT_MODE_STEAM){
                bool isExtracting = g_appData.machineState & MC_STATE_STEAM;
                if(isExtracting && !ack->getData(BUF_RCV_D3)){
                    g_appData.machineState &= ~MC_STATE_STEAM;
                    g_appData.statusChangeFlag |= CS_STEAM_DOWN;
                }
                LOGI("蒸汽完成");
            }else{
                bool isExtracting = g_appData.machineState & MC_STATE_EXT;
                if(isExtracting && !ack->getData(BUF_RCV_D3)){
                    g_appData.machineState &= ~MC_STATE_EXT;
                    g_appData.statusChangeFlag |= CS_EXTRACT_DOWN;
                }
            }
            
            break;
        }case CMD_ELECTRONIC_SCALE:{
            if(g_appData.scaleCalFlag != ack->getData(BUF_RCV_D3)){
                g_appData.scaleCalFlag = ack->getData(BUF_RCV_D3);
                g_appData.statusChangeFlag |= CS_SCALE_CAL_CHANGE;
            }
            if(std::abs(g_appData.scaleCalWeight) != (ack->getData(BUF_RCV_D5)<<8 | ack->getData(BUF_RCV_D6))){
                g_appData.scaleCalWeight = (ack->getData(BUF_RCV_D5)<<8 | ack->getData(BUF_RCV_D6))*(ack->getData(BUF_RCV_D4)?-1:1);
                g_appData.statusChangeFlag |= CS_SCALE_CAL_CHANGE;
            }
            break;
        }case CMD_ERROR:{
            // 第三字节先屏蔽，因为暂时未对第三字节的警告做处理，会出现弹窗取消不掉的情况
            if(g_appData.warnState != (ack->getData(BUF_RCV_D3) | ack->getData(BUF_RCV_D4)<<8/* | ack->getData(BUF_RCV_D5) << 16 */)){
                g_appData.warnState = (ack->getData(BUF_RCV_D3) | ack->getData(BUF_RCV_D4)<<8/* | ack->getData(BUF_RCV_D5) << 16 */);
                LOGV("g_appData.warnState = %x g_appData.warnIsDealState = %x",g_appData.warnState,g_appData.warnIsDealState);
                g_appData.warnIsDealState &= g_appData.warnState;
                g_appData.statusChangeFlag |= CS_WARN_STATUS_CHANGE;
                LOGV("g_appData.warnState = %x g_appData.warnIsDealState = %x",g_appData.warnState,g_appData.warnIsDealState);
            }
            reportErrorDataFrame();
            break;
        }case CMD_MAS_ESP:{
            bool isExtracting = g_appData.machineState & MC_STATE_EXT;
            if(isExtracting && !ack->getData(BUF_RCV_D3)){
                g_appData.machineState &= ~MC_STATE_EXT;
                g_appData.statusChangeFlag |= CS_EXTRACT_DOWN;
            }
            break;
        }case CMD_CLEAN_SELF:{
            if((g_appData.machineState & MC_STATE_CLEAN_WATER) && !ack->getData(BUF_RCV_D3)){
                g_appData.machineState &= ~MC_STATE_CLEAN_WATER;
                g_appData.statusChangeFlag |= CS_CLEAN_DOWN;
            }else if((g_appData.machineState & MC_STATE_CLEAN_CAL) && !ack->getData(BUF_RCV_D3)){
                g_appData.machineState &= ~MC_STATE_CLEAN_CAL;
                g_appData.statusChangeFlag |= CS_CLEAN_DOWN;
            }
            break;
        }case CMD_HAND_WASH:{
            bool isExtracting = g_appData.machineState & MC_STATE_EXT;
            if(isExtracting && !ack->getData(BUF_RCV_D3)){
                g_appData.machineState &= ~MC_STATE_EXT;
                g_appData.statusChangeFlag |= CS_EXTRACT_DOWN;
            }
            break;
        }case CMD_EXTRACTION_TEA:{
            bool isExtracting = g_appData.machineState & MC_STATE_EXT;
            if(isExtracting && !ack->getData(BUF_RCV_D3)){
                g_appData.machineState &= ~MC_STATE_EXT;
                g_appData.statusChangeFlag |= CS_EXTRACT_DOWN;
            }
            break;
        }case CMD_WORK_DATA:{
            if(g_appData.extractWater != (ack->getData(BUF_RCV_D4)*256+ack->getData(BUF_RCV_D5))){
                g_appData.extractWater = (ack->getData(BUF_RCV_D4)*256+ack->getData(BUF_RCV_D5));
                g_appData.statusChangeFlag |= CS_EXTRACT_DATA_CHANGE;
            }
            if(g_appData.extractPressure != ack->getData(BUF_RCV_D6)){
                g_appData.extractPressure = ack->getData(BUF_RCV_D6);
                g_appData.statusChangeFlag |= CS_EXTRACT_DATA_CHANGE;
            }
            break;
        }case CMD_DEVICE_SELF_TEST:{
            // 创建一个缓冲区来存储提取的子字符串
            char substring[16]; // 15 个字符 + 1 个 null 终止符
            strncpy(substring, (const char*)ack->mBuf + 3, 15);
            substring[15] = '\0'; // 添加 null 终止符
            g_appData.McuVersion = substring;
            LOGE("g_appData.McuVersion = %s  substring = %s",g_appData.McuVersion.c_str(),substring);
            if(!g_appData.mcuUpgradeVer.empty()&&(g_appData.McuVersion != g_appData.mcuUpgradeVer)){
                g_appData.statusChangeFlag |= CS_CHECK_OTA;
                g_windMgr->updateDate();
            }
            break;
        }case CMD_UPDATE_INFO:{
            if(ack->getData(BUF_RCV_D3)){
                g_appData.mcuOTAProgress = 0;
                sendBootLoaderData();
            }else{
                g_appData.mcuOTAProgress = 100;
                g_appData.mcuOTAError = "升级启动失败";
                g_appData.statusChangeFlag |= CS_MCU_OTA_ERROR;
                g_windMgr->updateDate();
            }
            break;
        }case CMD_UPDATE_DATA:{
            if(ack->getData(BUF_RCV_D3) == mSendOTACount-1){
                g_appData.mcuOTAProgress = ((float)mSendOTALen/mAllOTALen)*100;
                if(mSendOTALen >= mAllOTALen){
                    sendBootLoaderCheck();
                }else{
                    sendBootLoaderData();
                }
            }else{
                g_appData.mcuOTAProgress = 100;
                g_appData.mcuOTAError = "数据包错误:["+std::to_string(mSendOTACount)+"]"+std::to_string(mSendOTALen)+"/"+std::to_string(mAllOTALen);
                g_appData.statusChangeFlag |= CS_MCU_OTA_ERROR;
                g_windMgr->updateDate();
            }
            break;
        }case CMD_UPDATE_CHECK_END:{
            g_appData.mcuOTAProgress = 100;
            if(ack->getData(BUF_RCV_D3)){
                g_appData.mcuOTAError = "校验不通过";
                g_appData.statusChangeFlag |= CS_MCU_OTA_ERROR;
                g_windMgr->updateDate();
            }else{
                g_appData.statusChangeFlag |= CS_MCU_OTA_SUCCESS;
                g_windMgr->updateDate();
            }
            break;
        }
    }
    if(g_appData.statusChangeFlag) g_windMgr->updateDate();

}

void CConnMgr::checkMCUOTA() {
    std::string mFilePath;
    int n = g_appData.mcuUpgradeUrl.find_last_of('/');
    mFilePath = "/tmp/" + g_appData.mcuUpgradeUrl.substr(n + 1);
    
    std::ifstream file(mFilePath, std::ios::binary);

    if (!file.is_open()) {
        LOGE("Failed to open file: %s", mFilePath.c_str());
        return;
    }
    LOGW("Open ota file: %s", mFilePath.c_str());

    // mNeedOTA = true;

    // 获取文件大小
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // 转换文件大小类型
    mAllOTALen = static_cast<size_t>(fileSize);

    // 分配内存
    if (mSendOTAData)delete[] mSendOTAData;
    mSendOTAData = new uchar[mAllOTALen];
    mSendOTALen = 0;
    
    // mRedayOTA = false;

    // 读取文件内容到 data
    file.read(reinterpret_cast<char*>(mSendOTAData), fileSize);

    if (file.gcount() != fileSize) {
        LOGE("Failed to read ota file: %s", mFilePath.c_str());
        delete[] mSendOTAData;
        mSendOTAData = nullptr; // 避免悬空指针
        mAllOTALen = 0;
        // mNeedOTA = false;
        g_appData.mcuOTAError = "升级包数据异常";
        g_appData.statusChangeFlag |= CS_MCU_OTA_ERROR;
        g_windMgr->updateDate();
    } else {
        mOTADataSum = 0;
        for(int i=0; i<mAllOTALen; i++){
            mOTADataSum+=mSendOTAData[i];
        }
        mSendOTACount = 1;
        setBootLoaderInfo();
        LOGW("Ota file(%p) info:  size|%d  packetCount|%d mOTADataSum = %d", mSendOTAData, mAllOTALen, (mAllOTALen + 255) / 256,mOTADataSum);
    }

    file.close();
}