/*
 * @Author: hanakami
 * @Email: hanakami@163.com
 * @Date: 2025-10-08 16:10:56
 * @LastEditTime: 2025-10-23 14:47:17
 * @FilePath: /t5_ai_coffee/src/net/ota.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 * 
**/

#include "ota.h"

#include "conf_mgr.h"
#include <json/json.h>
#include <hv_version.h>
#include <project.h>
#include <cstdlib>
#include "ghc/filesystem.hpp"
#include <comm_class.h>
#include "wind_mgr.h"

void checkUpdata(){
    // CurlDownloader *mCurl = new CurlDownloader;
    CurlDownloader::ConnectionData* cnn = new CurlDownloader::ConnectionData("http://tft-oss-south.oss-cn-guangzhou.aliyuncs.com/ota/62525/vesion.json", "",
        [cnn](int code, char* body, std::string&) {
            if (code) {
                LOGE("fail to get new version ! code = %d", code);
                g_appData.isNeedUpdate  = false;
                g_appData.statusChangeFlag |= CS_CHECK_OTA;
                g_windMgr->updateDate();
                return;
            }
            if(body == nullptr){
                LOGE("fail to get new version ! code = %d  body is nullptr!!", code);
                g_appData.isNeedUpdate  = false;
                g_appData.statusChangeFlag |= CS_CHECK_OTA;
                g_windMgr->updateDate();
                return;
            }
            LOGD("success to get new version");
            Json::Value  root;
            Json::Reader reader(Json::Features::strictMode());
            bool bRet = reader.parse(body, root);
            if (!bRet) {
                LOGE("fail to get new version ! code = %d",bRet);
            } else {
                g_appData.isNeedUpdate  = false;
                if (!root["code"].isInt()) { 
                    LOGE("get new version data error !");
                    g_appData.statusChangeFlag |= CS_CHECK_OTA;
                    g_windMgr->updateDate();
                    return; 
                }
                int code = std::move(root["code"].asInt());
                if (code != 200) { 
                    LOGE("get new version data error !");
                    g_appData.statusChangeFlag |= CS_CHECK_OTA;
                    g_windMgr->updateDate();
                    return; 
                }
                g_appData.isBeta = false;
                
                if((checkVersion(std::move(root["data"]["lowest_version"].asString()),HV_SOFT_VER_EXT) > 0 )){
                    g_appData.isNeedUpdate  = true;
                    g_appData.otaVersion    = std::move(root["data"]["lowest_version"].asString());
                    g_appData.otaVersionDes = std::move(root["data"]["lowest_version_text"].asString());
                    g_appData.updateUrl     = std::move(root["data"]["lowest_url"].asString());
                    g_appData.otaMd5        = std::move(root["data"]["lowest_ota_md5"].asString());
                    LOG(INFO) << "v:" << std::move(root["data"]["lowest_version"].asString()) << " url:" << g_appData.updateUrl << std::endl;
                }else if(root["data"]["beta_uuid"].isMember(g_objConf->getTuyaAuthUUID())){
                    g_appData.isBeta = true;
                    if((checkVersion(std::move(root["data"]["beta_version"].asString()),HV_BETA_BER_EXT) > 0 )){
                        g_appData.isNeedUpdate  = true;
                        g_appData.otaVersion    = std::move(root["data"]["beta_version"].asString());
                        g_appData.otaVersionDes = std::move(root["data"]["beta_latest_text"].asString());
                        g_appData.updateUrl     = std::move(root["data"]["beta_url"].asString());
                        g_appData.otaMd5        = std::move(root["data"]["beta_ota_md5"].asString());
                        LOG(INFO) << " url:" << g_appData.updateUrl << std::endl;
                    }
                    LOG(INFO) << "beta_version:" << std::move(root["data"]["beta_version"].asString()) << std::endl;
                }else if((checkVersion(std::move(root["data"]["latest_version"].asString()),HV_SOFT_VER_EXT) > 0 )){
                    g_appData.isNeedUpdate  = true;
                    g_appData.otaVersion    = std::move(root["data"]["latest_version"].asString());
                    g_appData.otaVersionDes = std::move(root["data"]["latest_text"].asString());
                    g_appData.updateUrl     = std::move(root["data"]["latest_url"].asString());
                    g_appData.otaMd5        = std::move(root["data"]["latest_ota_md5"].asString());
                    LOG(INFO) << "v:" << std::move(root["data"]["latest_version"].asString()) << " url:" << g_appData.updateUrl << std::endl;
                }

                if(root["data"].isMember("mcu_upgrade")){
                    if(g_objConf->getTuyaAuthUUID() == "uuid3943244a5f9a0fc0"){
                        LOGI("深圳测试机器，只使用旧的MCU固件！");
                    }else if(g_appData.isBeta){
                        g_appData.mcuUpgradeVer = std::move(root["data"]["mcu_upgrade"]["beta_mcu_version"].asString());
                        g_appData.mcuUpgradeUrl = std::move(root["data"]["mcu_upgrade"]["beta_mcu_url"].asString());
                        g_appData.mcuUpgradeMd5 = std::move(root["data"]["mcu_upgrade"]["beta_mcu_md5"].asString());
                        g_appData.mcuUpgradeText = std::move(root["data"]["mcu_upgrade"]["beta_mcu_text"].asString());
                    }else{
                        g_appData.mcuUpgradeVer = std::move(root["data"]["mcu_upgrade"]["mcu_version"].asString());
                        g_appData.mcuUpgradeUrl = std::move(root["data"]["mcu_upgrade"]["mcu_url"].asString());
                        g_appData.mcuUpgradeMd5 = std::move(root["data"]["mcu_upgrade"]["mcu_md5"].asString());
                        g_appData.mcuUpgradeText = std::move(root["data"]["mcu_upgrade"]["mcu_text"].asString());
                    }
                }

                if(root["data"].isMember(g_objConf->getDeviceMac())){
                    std::string uuid = std::move(root["data"][g_objConf->getDeviceMac()]["uuid"].asString());
                    std::string key = std::move(root["data"][g_objConf->getDeviceMac()]["key"].asString());
                    LOGI("uuid = %s  key = %s",uuid.c_str(),key.c_str());
                    if(uuid != g_objConf->getTuyaAuthUUID()) g_objConf->setTuyaAuthCode(uuid,key);
                }    
                if(g_appData.isBeta != g_objConf->getBetaVersion()){
                    if(!g_appData.isBeta && (checkVersion(std::move(root["data"]["latest_version"].asString()),HV_SOFT_VER_EXT) > 0 )){
                        g_appData.isNeedUpdate  = true;
                        g_appData.otaVersion    = std::move(root["data"]["latest_version"].asString());
                        g_appData.otaVersionDes = std::move(root["data"]["latest_text"].asString());
                        g_appData.updateUrl     = std::move(root["data"]["latest_url"].asString());
                        g_appData.otaMd5        = std::move(root["data"]["latest_ota_md5"].asString());
                        LOG(INFO) << "v:" << std::move(root["data"]["latest_version"].asString()) << " url:" << g_appData.updateUrl << std::endl;
                    }
                }
                LOGE("g_appData.mcuUpgradeVer = %s",g_appData.mcuUpgradeVer.c_str());
                g_appData.statusChangeFlag |= CS_CHECK_OTA;
                g_windMgr->updateDate();
                g_objConf->setBetaVersion(g_appData.isBeta);
            }
            // delete cnn;
        }
    );

    g_objCurl->addConnection(cnn);
}

void upDateDownLo(int &flag,int &progress){
    flag = OTA_FLAG_DOWMING;
    std::string mFilePath,updateUrl;
    if(g_appData.isNeedUpdate){
        int n = g_appData.updateUrl.find_last_of('/');
        mFilePath = "/tmp/" + g_appData.updateUrl.substr(n + 1);
        updateUrl = g_appData.updateUrl;
    }else{
        int n = g_appData.mcuUpgradeUrl.find_last_of('/');
        mFilePath = "/tmp/" + g_appData.mcuUpgradeUrl.substr(n + 1);
        updateUrl = g_appData.mcuUpgradeUrl;
    }
    
    if (ghc::filesystem::exists(mFilePath)) {
        ghc::filesystem::remove(mFilePath);
        std::cout << "File deleted." << std::endl;
    }
    LOGI("开始下载");
    
    // CurlDownloader *mCurl = new CurlDownloader;
    CurlDownloader::ConnectionData* cnn = new CurlDownloader::ConnectionData(updateUrl, mFilePath,
        [mFilePath,&flag,cnn](int code, char* body, std::string&) mutable{
            LOGE("mFilePath = %s",mFilePath.c_str());
            if (code) {
                // mUpWin->close();
                flag = OTA_FLAG_ERROR;
                system(std::string("rm "+ mFilePath).c_str());
                LOGI("下载失败~~~！");
                LOGE("fail to download new version ! code = %d", code);
                return;
            }
            LOGD("success to download new version");
            ghc::filesystem::path pFile(mFilePath);
            if (!ghc::filesystem::exists(pFile) || !ghc::filesystem::is_regular_file(pFile) || ghc::filesystem::file_size(pFile) == 0) {
                // mUpWin->close();
                flag = OTA_FLAG_ERROR;
                system(std::string("rm "+mFilePath).c_str());
                LOGI("下载失败~~~！");
                LOGE("fail to download file !! code = %d", code);
                return;
            } else {
                if(g_appData.isNeedUpdate) flag = OTA_FLAG_SUCCESS;
                LOGI("下载成功~~~！");
                // mUpWin->setStatusText("正在解析更新包");
#ifdef CDROID_SIGMA
                ThreadPool::ins()->add(new otaUpDateThread, &flag,true);
#endif
            }
            // delete cnn;
        },[&progress](double dlnow, double dltotal){
            progress = (int)((dlnow/dltotal)*100);
            // LOGV("dlnow = %f/%f  progress = %d --- ",dlnow,dltotal,progress);
        },true
    );
    g_objCurl->addConnection(cnn);
}

void runUpdate(int &flag) {
    std::string mFilePath,updateUrl,fileMd5;
    if(g_appData.isNeedUpdate){
        int n = g_appData.updateUrl.find_last_of('/');
        mFilePath = "/tmp/" + g_appData.updateUrl.substr(n + 1);
        updateUrl = g_appData.updateUrl;
        fileMd5   = g_appData.otaMd5;
    }else{
        int n = g_appData.mcuUpgradeUrl.find_last_of('/');
        mFilePath = "/tmp/" + g_appData.mcuUpgradeUrl.substr(n + 1);
        updateUrl = g_appData.mcuUpgradeUrl;
        fileMd5   = g_appData.mcuUpgradeMd5;
    }

    // int n = g_appData.updateUrl.find_last_of('/');
    // std::string mFilePath = "/tmp/" + g_appData.updateUrl.substr(n + 1);
    std::string md5 = sysCommand("md5sum " + mFilePath + " | awk '{print $1}'");
    md5.erase(std::remove(md5.begin(), md5.end(), ' '), md5.end()); // 去除空格
    md5.erase(std::remove(md5.begin(), md5.end(), '\n'), md5.end()); // 去除换行符

    if(fileMd5 == md5){
        if(g_appData.isNeedUpdate){
            std::string command = "/customer/app/upgrade.sh";
            std::string BeforeUpdateCmd = "mount -o remount rw /customer;rm /customer/app/bakFiles/ -rf";
            std::system(BeforeUpdateCmd.c_str());

            if (std::system(command.c_str())) {
                system(std::string("rm "+mFilePath).c_str());
                flag = OTA_FLAG_UPDATE_ERROR;
                LOGE("fail upDate error %d",std::strerror(errno));
            };
        }else{
            flag = OTA_FLAG_MCU_SUCCES;
        }
    }else{
        system(std::string("rm "+mFilePath).c_str());
        flag = OTA_FLAG_MD5_ERROR;
        LOGE("fail! md5sum error!  md5 = %s",md5.c_str());
    }
}

int otaUpDateThread::onTask(void* data){
    int& flag = *static_cast<int*>(data);
    runUpdate(flag);
    return 0;
}

void otaUpDateThread::onMain(void *data) {
    int *dat = (int *)data;
}


