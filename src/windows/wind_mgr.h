#ifndef _WIND_MGR_H_
#define _WIND_MGR_H_

#include <widget/cdwindow.h>
#include "page_base.h"
#include "pop_base.h"
#include "page_keyboard.h"
#include "common.h"

typedef enum {
    CS_SETPAGE_SYNC_TIME        = 0x00000001,     // 设置页面的时间同步
    CS_LAMP_STATUS              = 0x00000002,     // 照明灯状态变更
    CS_WARN_CLOSE               = 0x00000004,     // 警告解除
    CS_WARN_STATUS_CHANGE       = 0x00000008,     // 警告状态变更

    CS_WIFI_ADAPTER_NOTIFI      = 0x00000010,     // wifi 列表更新
    CS_WIFI_CONNECT             = 0x00000020,     // wifi 连接状态更新

    CS_CHECK_OTA                = 0x00000040,     // 检测到OTA升级
    CS_NETWORK_CHANGE           = 0x00000080,     // wifi网络状态变更
    CS_WAAK_UP                  = 0x00000100,     // 休眠中唤醒屏幕

    CS_TUYA_NET_CHANGE          = 0x00000200,     // 涂鸦网络状态变更
    CS_PRE_HEATING              = 0x00000400,     // 预热状态变更

    CS_EXTRACT_DATA_CHANGE      = 0x00000800,     // 萃取的数据变化
    CS_GRIND_BEAN_DATA_CHANGE   = 0x00001000,     // 磨豆的数据变化
    CS_HOT_WATER_DOWN           = 0x00002000,     // 热水 完成
    CS_EXTRACT_DOWN             = 0x00004000,     // 萃取 完成
    CS_STEAM_DOWN               = 0x00008000,     // 蒸汽 完成
    CS_CLEAN_DOWN               = 0x00010000,     // 清洁完成

    CS_GET_AUTH_STATUS          = 0x00020000,     // 获取授权码状态变化
    CS_SCALE_CAL_CHANGE         = 0x00040000,     // 标定状态变化
    CS_MCU_OTA_ERROR            = 0x00080000,     // MCU更新失败
    CS_MCU_OTA_SUCCESS          = 0x00100000,     // MCU更新成功

    CS_TUYA_START_GRIND         = 0x00200000,     // 涂鸦的研磨指令下发
    CS_TUYA_START_EXT           = 0x00400000,     // 涂鸦的萃取、热水指令下发
    CS_TUYA_START_CLEAN         = 0x00800000,     // 涂鸦的清洁指令下发

    CS_AI_DATA_CHANGE           = 0x01000000,     // AI数据更新
    CS_AI_EXT                   = 0x02000000,     // AI发送萃取模式数据
}callbackStatus;



typedef enum {
    SC_NULL = 0,    // 无状态更新

    SC_TDS_STATE,               // 自定义信号传输 TDS（净水TDS）
    SC_PRUEWATER_TDS,           // 纯水TDS
    SC_INWATER_TEM,             // 进水温度、常温水温度变化
    SC_COOL_WATER_TEM,          // 制冷温度，冰水温度变化
    SC_HOT_WATER_TEM,           // 热水温度变化

    SC_WARN_CUSTOMER = 0x100,   // 自定义错误码
    SC_WARN_INTAKE_WATER_PROBE, // 进水温探头报警
    SC_WARN_ICE_WATER_PROBE,    // 冰水温探头报警
    SC_WARN_HOT_WATER_PROBE,    // 热水温探头报警
    SC_WARN_OVER_DRY_HEATING,   // 超温/干烧报警
    SC_WARN_MAKE_COOL,          // 制冷异常
    SC_WARN_MAKE_HOT,           // 制热异常
    SC_WARN_MAKE_ICE,           // 结冰报警
    SC_WARN_STORTAGE_WATER,     // 缺水报警

    SC_WARN_LEAKAGE_WATER,      // 漏水报警
    SC_WARN_LVXIN,              // 滤芯到期报警
    SC_WARN_FLOW,               // 流量传感器报警
    SC_WARN_COMM_FAIL,          // 通信故障(表示长时间未收到面板发往主板的信号)
    SC_UV_COMPLETE,             // UV功能完成
    SC_ICE_COMPLETE,            // 制冰完成(冰满)
    SC_WASH_COMPLETE,           // 冲洗完成
    SC_PIPE_BLOCKAGE,           // 管道阻塞报警
    SC_DRAIN_WATER_COMPLETE,    // 排水完成
    SC_OUTWATER_COUNT,          // 出水计数
    SC_LAMP_CHANGE,             // 出水灯状态变化
    SC_WARN,                    // 报警

    SC_OUT_WATER_COMPLETE = 0x200,  // 放水完成
    SC_OUT_ICE_COMPLETE,        // 出冰块完成
    SC_RO_RESET_COMPLETE,       // PP滤芯复位完成
    SC_HPCC_RESET_COMPLETE,       // RO滤芯复位完成
    
    SC_RO_PERCEN,                   // RO滤芯百分比
    SC_HPCC_PERCEN,                 // HPCC滤芯百分比

    SC_WIFI_ADAPTER_NOTIFI,     // wifi 列表更新
    SC_WIFI_CONNECT,            // wifi 连接成功
}appStatusChange;

#define g_windMgr CWindMgr::ins()

// 业务窗口管理
class CWindMgr:public MessageHandler{
#define USE_DELAY_RECLCYER 1
protected:
    enum{
        MSG_PAGE_DELAY_CLOSE=100, // 延迟关闭页面
    };
private:
    std::map<callbackStatus, appCb> mAppCallback; // 程序回调函数
    // std::map<Tuya_Cloud_Cmd, appCb> mTuyaCallback; // 涂鸦回调函数

    Window          *mWindow;       // 窗口指针
    ViewGroup       *mWindLayout;   // 根视图

    ViewGroup       *mWindPageBox;      // 页面Box  
    ViewGroup       *mWindPopBox;       // 弹窗Box
    ViewGroup       *mWindKeyBoardBox;  // 键盘box
    PageKeyBoard    *mKeyBoard;         // 键盘类
    PageBase        *mShowPage;         // 当前显示的页面
    PageBase        *mHomePage;         // 主页 的页面 不销毁
    PageBase        *mSettingPage;      // 设置 的页面 不销毁
    PageBase        *mPreActionPage;    // 休眠 的页面 
    PopBase         *mPopPage;          // 当前显示的弹窗页面

    Message         mPageCloseMesg; // 关闭页面消息

    std::vector<PageBase*> mClosePageList;  // 需要回收 的窗口列表
    std::vector<pageType>  mPageList;      // 窗口列表
public:
    static CWindMgr *ins(){
        static CWindMgr ins;
        return &ins;  
    }

    void initWindow(); // 初始化窗口
    void showPage(pageType type,bool isNewPage = true);
    void showPrevPage(); // 显示上一个页面
    void showHomePage(); // 显示首页
    void showPopPage(int type,View *gaussFromView,appCb enterCb = nullptr,appCb cencelCb = nullptr); // 显示弹窗

    void showSelectPosPopPage(int type,View *gaussFromView,int selectPos,popModeCb enterCb = nullptr,popModeCb cencelCb = nullptr);                 // 显示 只有选择一个pos的弹窗
    void showSndStepPopPage(int type,View *gaussFromView,int selectPos,int selectStep,popModeCb enterCb = nullptr,popModeCb cencelCb = nullptr);    // 显示 有二级步骤的弹窗

    void dealClosePopPage(bool autoClickEnterCb = false, bool autoClickCancelCb = false);    // 关闭弹窗
    void showKeyBoardPage(std::string iText,std::string description,int maxCount,input_callback enterCallback = nullptr,exit_callback exitCallback = nullptr); // 显示键盘页面
    void dealCloseKeyBoardPage(); // 关闭键盘页面

    void updateDate(); // 更改页面数据
    void changeTitleBar(uchar status); // 更改标题栏

    void handleMessage(Message& message)override;
private:
    void screenSaver(bool bEnabled); // 屏保
protected:
    CWindMgr(){}
    ~CWindMgr();
};

class WindowBase : public Window {
public:

    WindowBase();
    ~WindowBase();

};

#endif
