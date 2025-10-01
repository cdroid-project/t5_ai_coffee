#ifndef __proto_h__
#define __proto_h__

// 交互协议相关定义
////////////////////////////////////////////////////////////////////////////////////////////////
// MCU 发送给 UI  （ 主板 发送给 显示板 ）

typedef enum{
    BIT0 = 0x01,
    BIT1 = 0x02,
    BIT2 = 0x04,
    BIT3 = 0x08,
    BIT4 = 0x10,
    BIT5 = 0x20,
    BIT6 = 0x40,
    BIT7 = 0x80,
    BIT8 = 0x0100,
    BIT9 = 0x0200,
    BIT10 = 0x0400,
    BIT11 = 0x0800,
    BIT30 = 0x40000000,
    BIT31 = 0x80000000,
} CmdBit;

// 命令字
enum{
    CMD_SHUTDOWN                    = 0x01,     // 关机
    CMD_PRE_HEATING                 = 0x02,     // 预热
    CMD_GRIND_BEAN                  = 0x03,     // 磨豆
    CMD_EXTRACTION                  = 0x04,     // 萃取
    CMD_ELECTRONIC_SCALE_CAL        = 0x05,     // 电子秤校准
    CMD_ELECTRONIC_SCALE            = 0x06,     // 电子秤
    
    CMD_ERROR                       = 0x08,     // 异常上报
    CMD_BEEP                        = 0x09,     // 蜂鸣器

    CMD_UPDATE_INFO                 = 0x0A,     // bootLoader 升级信息下发
    CMD_UPDATE_DATA                 = 0x0B,     // 固件下发指令（升级数据）
    CMD_UPDATE_CHECK_END            = 0x0C,     // 固件下发校验结果查询指令

    CMD_DEVICE_SELF_TEST            = 0x0D,     // 设备自检
    CMD_CLEAN_SELF                  = 0x0E,     // 自清洁（除垢）

    CMD_MAS_ESP                     = 0x0F,     // 大师浓缩
    CMD_HAND_WASH                   = 0x10,     // 手冲咖啡
    CMD_EXTRACTION_TEA              = 0x11,     // 萃茶
    CMD_WORK_DATA                   = 0x12,     // 工作数据
    CMD_DEVICE_SET                  = 0x13,     // 机器设置指令
} ;

// 萃取模式
enum{
    EXT_MODE_ESP            = 0x01,     // 意式咖啡
    EXT_MODE_ESP_DOUBLE     = 0x02,     // 意式咖啡 (双杯)
    EXT_MODE_AMERICANO      = 0x03,     // 美式咖啡
    EXT_MODE_HOT_WATER      = 0x04,     // 热水
    EXT_MODE_STEAM          = 0x05,     // 蒸汽
};

// 清洁模式
enum{
    CLEAN_TYPE_WATER        = 0x01,     // 水路清洁
    CLEAN_TYPE_CALCIFY      = 0x02,     // 钙化清洁
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// DP 的ID枚举
typedef enum{
    TYCMD_SWITCH                    = 1,  // 开关                          布尔型
    TYCMD_BEAN_START                = 2,  // 磨豆启动/停止                  布尔型
    TYCMD_WORK_STATE                = 4,  // 工作状态             只上报    枚举型
    TYCMD_MODE                      = 5,  // 咖啡模式                       枚举型
    TYCMD_CUP_NUMBER                = 13, // 杯数选择                       枚举型

    TYCMD_TEMP_CURRENT              = 16, // 当前温度             只上报    数值型          

    TYCMD_POWDER_SET                = 19, // 粉量设置                      数值型 
    TYCMD_FAULT                     = 22, // 故障告警             只上报    故障型
    
    TYCMD_CLEAN_PIPE                = 101,// 水路清洁                      布尔型
    TYCMD_CLEAN_CAL                 = 102,// 钙化清洁                      布尔型
    TYCMD_MAKE_TYPE                 = 103,// 制作大类型                    枚举型      
    TYCMD_COFFEE_DIY                = 104,// 咖啡DIY                       透传型
    TYCMD_TEA_DIY                   = 105,// 萃茶IDY                       透传型
    TYCMD_BREW_AGAIN                = 106,// 再次冲泡                      数值型
    TYCMD_NUM_EXTRACT_TEA           = 107,// 萃茶次数             只上报    数值型
    TYCMD_NUM_GRIND                 = 108,// 研磨次数             只上报    数值型
    TYCMD_CLEAN_CAL_SECOND          = 109,// 钙化二次清洁                   布尔型
    TYCMD_CLEANING_START            = 110,// 清洁启动/暂停
    TYCMD_CLEANING_MODE             = 111,// 清洁工作模式                  枚举型
    TYCMD_GRAIN_UNIT_MODE           = 112,// 磨豆模式                      枚举型

    TYCMD_WORK_ALARM                = 114,// 异常提醒             只上报   故障型  
    TYCMD_NUM_ESPRESSO              = 118,// 意式咖啡杯数         只上报   数值型
    TYCMD_NUM_AMERICANO             = 119,// 美式咖啡杯数         只上报   数值型
    TYCMD_NUM_MASTER                = 120,// 大师咖啡杯数         只上报   数值型
    TYCMD_NUM_POUR                  = 121,// 手冲咖啡杯数         只上报   数值型
    TYCMD_NUM_HOT                   = 123,// 出热水次数           只上报   数值型
    TYCMD_NUM_MILK                  = 124,// 打奶泡次数           只上报   数值型
    TYCMD_TEA_MODE                  = 125,// 萃茶模式                     枚举型
    TYCMD_MAKEING_START             = 126,// 制作停止/启动                 布尔型

    
}Tuya_Cloud_Cmd;

// 设备状态
typedef enum{
    ES_STANDBY              = 0x00,     // 正常工作状态(待机)
    ES_GRINDING             = 0x01,     // 研磨中
    ES_EXTRACTING           = 0x02,     // 萃取中
    ES_GRIND_DONE           = 0x03,     // 研磨完成
    ES_EXTRACT_DONE         = 0x04,     // 萃取完成
    ES_PRE_HEATING          = 0x05,     // 测试中
    ES_EASHING              = 0x06,     // 洗茶中
    ES_EASH_DONE            = 0x07,     // 洗茶完成
    ES_CLEAN_PIPE           ,           // 水路清洁中
    ES_CLEAN_CAL_1          ,           // 第一次钙化清洁中
    ES_CLEAN_CAL_1_DONE     ,           // 第一次钙化清洁完成
    ES_CLEAN_CAL_2          ,           // 第二次钙化清洁中
    ES_CLEAN_CAL_DONE       ,           // 清洁完成
    ES_RESET                ,           // 重置中（OTA）
    ES_SLEEPING             ,           // 休眠中
}EquipmentStatus;

// 制作大类型
enum{
    MAKE_TYPE_COFFEE = 0,   // 咖啡
    MAKE_TYPE_TEA           // 萃茶
};

// 咖啡模式
enum{
    CFM_ESP = 1,                // 意式咖啡
    CFM_AMERICANO,              // 美式咖啡
    CFM_HOT,                    // 热水
    CFM_MAS_CLASSIC_9_BAR,      // 大师浓缩 经典-9bar
    CFM_MAS_GENTLE_AND_SWEET,   // 大师浓缩 温和香甜
    CFM_MAS_RICH_AND_SWEET,     // 大师浓缩 浓郁回甘 
    CFM_MAS_CLASSIC_ITALIAN,    // 大师浓缩 经典意式浓缩
    CFM_MAS_TURBO_SHOT,         // 大师浓缩 Turbo Shot
    CFM_MAS_LUNGO,              // 大师浓缩 Lungo
    CFM_MAS_CUSTOMER,           // 大师浓缩 自定义

    CFM_HAND_WASH_CLASSIC_TREE, // 手冲咖啡 经典三刀流
    CFM_HAND_WASH_ONE_BLADE_SCHOOL, // 手冲咖啡 一刀流
    CFM_HAND_WASH_DEEP_BAKING,  // 深度烘焙
    CFM_HAND_WASH_MEDIUM_ROAST, // 中度烘焙
    CFM_HAND_WASH_SHALLOW_BAKING,//浅度烘焙
    CFM_HAND_WASH_CUSTOMER,     // 自定义
};

// 咖啡大类型
enum{
    COFFEE_TYPE_NORMAL = 1, // 常规萃取
    COFFEE_TYPE_MAS,        // 大师浓缩
    COFFEE_TYPE_HANDWASH,   // 手冲咖啡
};

// 流速
enum{
    FLOW_RATE_LOW = 1,      // 低
    FLOW_RATE_MEDIUM,       // 中
    FLOW_DATE_HIGH,         // 高
};

// 萃茶类型
enum{
    TEA_TYPE_FLOWER = 1,    // 花茶
    TEA_TYPE_GREEN,         // 绿茶
    TEA_TYPE_YELLOW,        // 黄茶
    TEA_TYPE_WHITE,         // 白茶 
    TEA_TYPE_CYAN,          // 青茶
    TEA_TYPE_RED,           // 红茶
    TEA_TYPE_BLACK,         // 黑茶
};

// 萃茶大类型
enum{
    EXTRACTING_TEA = 1, // 萃茶
    MAKE_TEA,           // 冲茶
};

// 清洁工作模式
enum{
    CLEAN_MODE_PIPE         = 0, // 水路清洁
    CLEAN_MODE_CAL          = 1, // 钙化清洁
    CLEAN_MODE_CAL_SECOND   = 2, // 钙化二次清洁
};

// 电子秤标定状态枚举
enum{
    SCALE_CAL_CHANGE_POINT  = 0x00, // 开机找零点
    SCALE_CAL_GET_WEIGHT    = 0x01, // 获取重量
    SCALE_CAL_CLEAR_WEIGHT  = 0x02, // 重量清零
    SCALE_CAL_ZERO          = 0x03, // 标定0点
    SCALE_CAL_500G          = 0x04, // 标定500g
    SCALE_CAL_1000G         = 0x05, // 标定1000g
    SCALE_CAL_SUCCESS       = 0x06, // 标定成功
    SCALE_CAL_FAIL          = 0x07, // 标定失败
};

// 故障警告
enum{
    WARN_NONE = 0x0000, 
    E01                     = 0b0000000000000001,    // (bit0) 咖啡机传感器高温保护（未用上）
    E15                     = 0b0000000000000010,    // (bit1) 蒸汽传感器高温保护（未用上）
    E16                     = 0b0000000000000100,    // (bit2) 出奶传感器高温保护（未用上）
    E02                     = 0b0000000000001000,    // (bit3) 咖啡传感器异常
    E03                     = 0b0000000000010000,    // (bit4) 蒸汽传感器异常
    E04                     = 0b0000000000100000,    // (bit5) 出奶传感器异常
    E05                     = 0b0000000001000000,    // (bit6) 流量计异常
    E06                     = 0b0000000010000000,    // (bit7) 通讯异常
    E09                     = 0b0000000100000000,    // (bit8) 工作中取走容器
    E10                     = 0b0000001000000000,    // (bit9) 水箱未装配 

    A01                     = 0b0000010000000000,    // (bit10) 缺水异常
    A02                     = 0b0000100000000000,    // (bit11) 除垢提醒（使用累计1500次）
    E07                     = 0b0001000000000000,    // (bit12) 豆盒未装配

    E13                     = 0b0010000000000000,    // (bit13) 过零线路异常（未用上）
    E08                     = 0b0100000000000000,    // (bit14) 磨豆电机保护
    A03                     = 0b1000000000000000,    // (bit15) 萃取压力过大

    NEED_STOP_WORK_ERROR    = 0b0110001011111111,   // 需要停止工作的警告

    STOP_GRIND_BEAN_ERROR   = 0b0111000110000111,   // 磨豆时，需要停止工作的警告
    STOP_EXT_ERROR          = 0b0010011111101111,   // 萃取时，需要停止工作的警告
    STOP_HOT_WATER_ERROR    = 0b0010011011000111,   // 出热水时，需要停止工作的警告
    STOP_STEAM_ERROR        = 0b0010011010010111,   // 出蒸汽时，需要停止工作的警告
};





#endif
