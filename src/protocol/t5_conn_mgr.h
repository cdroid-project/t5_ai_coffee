
#ifndef __t5_conn_mgr_h__
#define __t5_conn_mgr_h__

#include "packet_buffer.h"
#include "uart_client.h"
#include "cmd_handler.h"
#include "data.h"

typedef std::function<void()> factoryCb;
#define g_T5Mgr T5Mgr::ins()
class T5Mgr : public EventHandler, public IHandler {
protected:
    T5Mgr();
    ~T5Mgr();

public:
    static T5Mgr *ins() {
        static T5Mgr stIns;
        return &stIns;
    }

    int init();
    // 发送给按键板
    void send2MCU();
    

protected:
    virtual int checkEvents();
    virtual int handleEvents();
    virtual void onCommDeal(IAck *ack);
private:
    IPacketBuffer   *mPacket;
    int64_t          mNextEventTime;
    int64_t          mLastSendTime;
    UartClient      *mUartMCU; // mcu
    int              mSetData; // 设置数据次数
    uchar            mSndData[UI2MCU::BUF_LEN]; // 发送缓冲区

    std::string      mT5RecvText;
    bool             mIsT5RecvEnd;
};



#endif
