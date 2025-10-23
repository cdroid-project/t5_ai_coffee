/*
 * @Author: cy
 * @Email: 964028708@qq.com
 * @Date: 2025-10-01 10:28:26
 * @LastEditTime: 2025-10-23 14:51:30
 * @FilePath: /t5_ai_coffee/src/common/json_func.cc
 * @Description: 
 * @BugList: 
 * 
 * Copyright (c) 2025 by cy, All Rights Reserved. 
 * 
**/


#include "json_func.h"
#include <cdlog.h>

#include <iostream>
#include <fstream>
#include <ghc/filesystem.hpp>

bool convertStringToJson(const std::string& str, Json::Value& root) {
    Json::Reader reader(Json::Features::strictMode());
    if (reader.parse(str, root)) {
        return true;
    }
    LOGE("Json format error!!!!");
    return false;
}

bool convertJsonToString(const Json::Value& root, std::string& str) {
    Json::StreamWriterBuilder writer;
    str = Json::writeString(writer, root);
    return true;
}

bool loadLocalJson(const std::string& filePath, Json::Value& root) {
    // 文件不存在或者是一个目录
    ghc::filesystem::path  checkFilePath(filePath);
    if (!ghc::filesystem::exists(checkFilePath) || ghc::filesystem::is_directory(checkFilePath)) {
        LOGE("Filepath no exists.");
        return false;
    }

    // 打开文件
    std::ifstream  pfile;
    pfile.open(filePath, std::ios::binary);
    if (!pfile.is_open()) {
        LOGE("Can not open %s", filePath.c_str());
        return false;
    }

    // 获取文件大小
    pfile.seekg(0, std::ios::end);
    std::streampos fileSize = pfile.tellg();
    pfile.seekg(0, std::ios::beg);
    pfile.clear();

    // 转储文件 
    char data[1024];
    std::string content;
    do {
        pfile.read(data, sizeof(data));
        size_t extracted = pfile.gcount();
        content.append(data, extracted);
    } while (!pfile.eof());
    pfile.close();

    LOGD("load json file path = %s",filePath.c_str());
    return convertStringToJson(content, root);
}

bool saveLocalJson(const std::string& filePath, const Json::Value& root) {
    // 路径检测
    ghc::filesystem::path  checkFilePath(filePath);
    if (ghc::filesystem::exists(checkFilePath)) {
        if (ghc::filesystem::is_regular_file(checkFilePath)) {
            // 如果是同名文件则删除文件
            ghc::filesystem::remove(checkFilePath);
        } else if (ghc::filesystem::is_directory(checkFilePath)) {
            LOGE("A directory with the same name already exists.");
            return false;
        }
    }

    // 检测文件是否打开成功
    std::ofstream outputFile(filePath);
    if (!outputFile.is_open()) {
        LOG(ERROR) << "Failed to open output file for writing";
        return false;
    }

    // 检测json是否转换成功
    std::string jsonString;
    if (!convertJsonToString(root, jsonString)) {
        LOG(ERROR) << "Failed to transion json to string";
        return false;
    }

    // 保存文件
    outputFile << jsonString;
    outputFile.close();
    // int fileFd = outputFile.rdbuf()->pubseekoff(0,outputFile.beg,outputFile.out);
    // // 获取文件描述符并刷新数据到磁盘
    // int fileDescriptor = outputFile.rdbuf()->pubseekoff(0, outputFile.beg, outputFile.out);
    // if (fileDescriptor != -1) {
    //     if (fsync(fileDescriptor) == -1) {
    //         LOG(ERROR) << "Failed to flush data to disk";
    //         return false;
    //     }
    // } else {
    //     LOG(ERROR) << "Failed to get file descriptor";
    //     return false;
    // }
#ifdef CDROID_SIGMA
    sync();
#endif
    return true;

}

int getJsonInt(const Json::Value& root, const std::string& key, int defaultValue) {
    if (!root.isMember(key)) {
        LOG(WARN) << "not find key:" << key;
        return defaultValue;
    }
    return jsonToInt(root[key], defaultValue);
}

std::string getJsonString(const Json::Value& root, const std::string& key, const std::string& defaultValue) {
    if (!root.isMember(key)) {
        LOG(WARN) << "not find key:" << key;
        return defaultValue;
    }
    return jsonToString(root[key], defaultValue);
}

bool getJsonBool(const Json::Value& root, const std::string& key, bool defaultValue) {
    if (!root.isMember(key)) {
        LOG(WARN) << "not find key:" << key;
        return defaultValue;
    }
    return jsonToBool(root[key], defaultValue);
}

int jsonToInt(const Json::Value& value, int defaultValue) {
    if (value.isInt())return value.asInt();
    if (value.isString()) return atoll(value.asCString());
    LOGD("value type error");
    return defaultValue;
}

std::string jsonToString(const Json::Value& value, const std::string& defaultValue) {
    if (value.isString()) return value.asString();
    LOGD("value type error");
    return defaultValue;
}

bool jsonToBool(const Json::Value& value, bool defaultValue) {
    if (value.isBool()) return value.asBool();
    if (value.isInt()) return value.asInt();
    if (value.isString()) return value.asString() == std::string("true");
    LOGD("value type error");
    return defaultValue;
}

bool areJsonValuesEqual(const Json::Value& val1, const Json::Value& val2) {
    // 判断类型是否相同
    if (val1.type() != val2.type()) {
        return false;
    }

    // 对象类型
    if (val1.isObject()) {
        if (val1.size() != val2.size()) {
            return false;
        }

        for (const auto& key : val1.getMemberNames()) {
            if (!val2.isMember(key) || !areJsonValuesEqual(val1[key], val2[key])) {
                return false;
            }
        }

        return true;
    }

    // 数组类型
    if (val1.isArray()) {
        if (val1.size() != val2.size()) {
            return false;
        }

        for (int i = 0; i < val1.size(); ++i) {
            if (!areJsonValuesEqual(val1[i], val2[i])) {
                return false;
            }
        }

        return true;
    }

    // 其他类型
    return val1 == val2;
}