#ifndef MYCANTHREAD_H
#define MYCANTHREAD_H

#include "ControlCAN.h"
#include <QThread>
#include <QCheckBox>
#include <QFile>

typedef enum {
    DIPRO_UNKNOWN                     = 0,                       /* 未知 */
    DIPRO_UDS,
    DIPRO_CCP,
    DIPRO_XCP
} DI_PROTOCOL_E;

typedef enum {
    UDS_SESSION_CTR                    = 0x10,                       /* 诊断会话控制 */
    UDS_ECU_RESET                      = 0x11,                       /* 电控单元复位 */
    UDS_CLR_DIAGNOSTIC_INFO            = 0x14,                       /* 清除诊断消息 */
    UDS_READ_DTC_INFO                  = 0x19,                       /* 读取DTC信息 */
    UDS_READ_DATA_BYID                 = 0x22,                       /* 读取数据 */
    UDS_READ_MEMORY                    = 0x23,                       /* 读取内存 */
    UDS_SECURITY_ACCESS                = 0x27,                       /* 安全访问 */
    UDS_COM_CTR                        = 0x28,                       /* 通讯控制 */
    UDS_READ_DATA_PERIODID             = 0x2A,                       /* 读取数据(周期标识符) */
    UDS_DYN_DEFINE_ID                  = 0x2C,                       /* 动态定义数据标识符 */
    UDS_WRITE_DATA_BYID                = 0x2E,                       /* 写入数据 */
    UDS_IO_CTR_BYID                    = 0x2F,                       /* 输入输出控制 */
    UDS_ROUTINE_CTR                    = 0x31,                       /* 例程控制 */
    UDS_REQ_DOWNLOAD                   = 0x34,                       /* 请求下载 */
    UDS_TRANSFER_DATA                  = 0x36,                       /* 数据传输 */
    UDS_REQ_TRANSFER_EXIT              = 0x37,                       /* 请求退出数据传输 */
    UDS_WRITE_FINGER_PRINT             = 0x3B,                       /* 写指纹 */
    UDS_WRITE_MEMORY                   = 0x3D,                       /* 写入内存 */
    UDS_TESTER_PRESENT                 = 0x3E,                       /* 诊断设备在线 */
    UDS_CTR_DTCSETTING                 = 0x85,                       /* 控制DTC设置 */
    UDS_RW_INFO                        = 0xBF                        /* 写入或读取部分信息 */
} UDS_SERVICE_E;

/* CCP协议命令代码 */
typedef enum {
    CCP_CONNECT             = 0x01,     /* Connect                                      连接 */
    CCP_SET_MTA             = 0x02,     /* Set Memory Transfer Address                  设置内存传输地址 */
    CCP_DNLOAD              = 0x03,     /* Data Download                                数据下载 */
    CCP_UPLOAD              = 0x04,     /* Data Upload                                  数据上传 */
    CCP_TEST                = 0x05,     /* Test Availability                            测试可用性 */
    CCP_START_STOP          = 0x06,     /* Start/Stop Data Transmission                 开始/停止通信 */
    CCP_DISCONNECT          = 0x07,     /* Disconnect                                   断开连接 */
    CCP_START_STOP_ALL      = 0x08,     /* Start/Stop Synchronised Data Transmission    开始/停止同步数据传输 */
    CCP_GET_ACTIVE_CAL_PAGE = 0x09,     /* Get Currently Active Calibration Page        获得当前活动的标定页 */
    CCP_SET_S_STATUS        = 0x0C,     /* Set Session Status                           设置周期状态 */
    CCP_GET_S_STATUS        = 0x0D,     /* Get Session Status                           得到周期状态 */
    CCP_BUILD_CHECKSUM      = 0x0E,     /* Build Checksum                               建立校验和 */
    CCP_SHORT_UP            = 0x0F,     /* Short Upload                                 短的上传 */
    CCP_CLEAR_MEMORY        = 0x10,     /* Clear Memory                                 清除内存 */
    CCP_SELECT_CAL_PAGE     = 0x11,     /* Select Calibration Data Page                 选择标定数据页 */
    CCP_GET_SEED            = 0x12,     /* Get Seed for Key                             获取种子 */
    CCP_UNLOCK              = 0x13,     /* Unlock                                       解除保护&解锁 */
    CCP_GET_DAQ_SIZE        = 0x14,     /* Get Size of DAQ List                         得到DAQ列表的尺寸大小 */
    CCP_SET_DAQ_PTR         = 0x15,     /* Set DAQ List Pointer                         设置DAQ列表指针 */
    CCP_WRITE_DAQ           = 0x16,     /* Write DAQ List Entry                         写DAQ列表入口 */
    CCP_EXCHANGE_ID         = 0x17,     /* Exchange Station Identifications             交换位置识别 */
    CCP_PROGRAM             = 0x18,     /* Program                                      编程 */
    CCP_MOVE                = 0x19,     /* Move Memory Block                            移动内存块 */
    CCP_GET_VERSION         = 0x1B,     /* Get Implemented Version of CCP               获得执行的CCP版本 */
    CCP_DIAG_SERVICE        = 0x20,     /* Diagnostic Service                           诊断服务 */
    CCP_ACTION_SERVICE      = 0x21,     /* Action Service                               动作服务 */
    CCP_PROGRAM_6           = 0x22,     /* Program 6 Bytes                              编程6字节 */
    CCP_DNLOAD_6            = 0x23,     /* Data Download 6 Bytes                        数据下载6字节 */

    MAX_CCP_CMD
} CCP_CMD_E;


/*这个类主要用来接收和发送can总线数据*/
class MyCanThread : public QThread
{
    Q_OBJECT

public:
    enum {
            ERROR_UNKOWN = 0,
            ERROR_SEND,
            ERROR_RECV,
            ERROR_OPEN_DEVICE,
            ERROR_CANINIT_DEVICE,
            ERROR_STARTUP_DEVICE,
            ERROR_CLOSE_DEVICE
        };
    MyCanThread();
    MyCanThread(int type, int device, int chanel);
    volatile bool m_throwrecvsignal;
    bool controlCanDevice(bool onoff);
    bool startupCanDevice(void);
    void setDeviceParam(uint accCode, uint accMask, int timingid, 
        int sendType, int remoteFlag, int externFlag);
    void setDeviceCanid(uint reqcanid, uint rspcanid);
    void setProtocol(int protocol);
    void setAutoResponse(int response);
    void setDeviceInfo(int type, int device, int chanel);
    void setShowTimeParam(bool showrelativetime);
    void setShowHexParam(bool showhexupper);
    void getFrameCount(uint &recvCount, uint &sendCount);
    void setFrameCount(uint recvCount, uint sendCount);
    void getTimerStr(int index, QString &tim0, QString &tim1);
    void startupAutoDiagI(uint32_t startCANID, const BYTE *data, uint8_t datalen);
    void setThrowRecvSignal(bool throwrecvsignal) { m_throwrecvsignal = throwrecvsignal; }
    void setFilterParam(int currentMode, const uint32_t *fromLowHex, 
                            const uint32_t *toHighHex, int len);
    void transferOtaData(const BYTE *data, BYTE datalen);
    
protected:
    void run();
private:
    DWORD m_devType;
    DWORD m_device;
    DWORD m_chanel;
    DWORD m_accCode;
    DWORD m_accMask;
    UCHAR m_timingid;
    BYTE m_sendType;
    BYTE m_remoteFlag;
    BYTE m_externFlag;
    VCI_CAN_OBJ sendObj;

    int m_protocol;

    uint m_recvFrameCount = 0;
    uint m_sendFrameCount = 0;

    QFile m_file;
    bool m_autoresponse = false;
    uint m_reqcanid = 0;
    uint m_rspcanid = 0;
    bool m_streamcontrol;
    bool m_genaratepacket;
    bool m_withcutline;
    
    uint8_t m_streaminterval;

    uint8_t m_ccpresuse;
    uint8_t m_ccpressave;
    
    bool m_showhexupper = false;
    bool m_showrelativetime = false;

    int m_hexLenght;
    int m_currentMode = 0;
    uint32_t m_fromLowHex[100];
    uint32_t m_toHighHex[100];

    QString analysisCanInfo(bool issend, const VCI_CAN_OBJ *info);
    void UDS_ecmHandle(uint canid, const BYTE *data, BYTE datalen);
    void CCP_ecmHandle(uint canid, const BYTE *data, BYTE datalen);
    void XCP_ecmHandle(uint canid, const BYTE *data, BYTE datalen);
    void sendCanData(bool iswithcutline, uint canid, const BYTE *data, BYTE datalen);
public slots:
    void slotStreamControl(int);
    void slotGenaratePacket(int);
    void slotWithCutline(int);
    void slotStreamInterval(const QString &value);
    void slotCcpResource(int resUse, int resSave);
signals:
    void showInfoSignal(const QString &info);
    void errorInfoSignal(int error, const QString &extend);
};
#endif // MYCANTHREAD_H
