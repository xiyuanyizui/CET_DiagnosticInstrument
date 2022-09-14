#include "mycanthread.h"
#include <QDateTime>
#include <QMessageBox>
#include <QIODevice>

#include <QDir>
#include <QDebug>

#define IS_ENABEL_RECV_ERROR_MONITOR  1

/* 该值已调试好 不需要改 改大会出 SID命令没有按顺序打印 */
#define THREAD_SLEEP_US 20

#define RECV_MAX_NUM 500

static const QString g_tim0[10]=
{
    "00",       // 1000Kbps
    "00",       // 800Kbps
    "00",       // 500Kbps
    "01",       // 250Kbps
    "03",       // 125Kbps
    "04",       // 100Kbps
    "09",       // 50Kbps
    "18",       // 20Kbps
    "31",       // 10Kbps
    "BF"        // 5Kbps
};
    
static const QString g_tim1[10]=
{
    "14",       // 1000Kbps
    "16",       // 800Kbps
    "1C",       // 500Kbps
    "1C",       // 250Kbps
    "1C",       // 125Kbps
    "1C",       // 100Kbps
    "1C",       // 50Kbps
    "1C",       // 20Kbps
    "1C",       // 10Kbps
    "FF"        // 5Kbps	
};

MyCanThread::MyCanThread() :
        QThread()
{
}

MyCanThread::MyCanThread(int type, int device, int chanel) :
    QThread()
{
    this->m_devType = type;
    this->m_device  = device;
    this->m_chanel  = chanel;
}

void MyCanThread::transferOtaData(const BYTE *data, BYTE datalen)
{
    QByteArray ba;
    for (int i = 0; i < datalen; ++i) {
        ba.append(data[i]) ;
    }
    m_file.write(ba);
    m_file.flush();
}

bool MyCanThread::controlCanDevice(bool onoff)
{
    if (onoff) {
        if (STATUS_OK != VCI_OpenDevice(m_devType, m_device, m_chanel)) {
            emit errorInfoSignal(ERROR_OPEN_DEVICE, QString());
            return false;
        }
    } else {
        if (STATUS_OK != VCI_CloseDevice(m_devType, m_device)) {
            emit errorInfoSignal(ERROR_CLOSE_DEVICE, QString());
            return false;
        } 
    }

    return true;
}

bool MyCanThread::startupCanDevice(void)
{
    bool ok = false;
    VCI_INIT_CONFIG config;

    config.AccCode = m_accCode;
    config.AccMask = m_accMask;
    config.Filter = 1;
    config.Mode = 0;
    config.Timing0 = g_tim0[m_timingid].toInt(&ok, 16);
    config.Timing1 = g_tim1[m_timingid].toInt(&ok, 16);

    qDebug("m_devtype:%lu, m_device:%lu, m_chanel:%lu", m_devType, m_device, m_chanel);
    qDebug("AccCode:0x%08X, AccMask:0x%08X, Filter:%u, Mode:%u, Timing0:0x%02X, Timing1:0x%02X",
        (uint)config.AccCode, (uint)config.AccMask, config.Filter, 
        config.Mode, config.Timing0, config.Timing1);
    
    if (STATUS_OK != VCI_InitCAN(m_devType, m_device, m_chanel, &config)) {
        emit errorInfoSignal(ERROR_CANINIT_DEVICE, QString());
        return false;
    }

    if (STATUS_OK != VCI_StartCAN(m_devType, m_device, m_chanel)) {
        emit errorInfoSignal(ERROR_STARTUP_DEVICE, QString());
        return false;
    }

    return true;
}

void MyCanThread::setDeviceInfo(int type, int device, int chanel)
{
    this->m_devType = type;
    this->m_device  = device;
    this->m_chanel  = chanel;
}

void MyCanThread::setDeviceParam(uint accCode, uint accMask, int timingid, 
        int sendType, int remoteFlag, int externFlag)
{
    this->m_accCode  = accCode;
    this->m_accMask  = accMask;
    this->m_timingid = timingid;
    this->m_sendType = sendType;
    this->m_remoteFlag = remoteFlag;
    this->m_externFlag = externFlag;
}

void MyCanThread::setDeviceCanid(uint reqcanid, uint rspcanid)
{
    this->m_reqcanid = reqcanid;
    this->m_rspcanid = rspcanid;
}

void MyCanThread::setShowTimeParam(bool showrelativetime)
{
    this->m_showrelativetime = showrelativetime;
}

void MyCanThread::setShowHexParam(bool showhexupper)
{
    this->m_showhexupper = showhexupper;
}

static QString timeStampToTimeStr(UINT TimeStamp)
{
    QString resultStr = "";

    int hour = TimeStamp / 36000000;
    int minute = (TimeStamp - hour * 36000000) / 600000;
    int second = (TimeStamp - hour * 36000000 - minute * 600000) / 10000;
    int ms = (TimeStamp - hour * 36000000 - minute * 600000 - second * 10000) / 10;
    int mms = (TimeStamp - hour * 36000000 - minute * 600000 - second * 10000 - ms * 10);

    resultStr = QString("%1:").arg(hour, 2, 10, QChar('0'));//时
    resultStr += QString("%1:").arg(minute, 2, 10, QChar('0'));//分
    resultStr += QString("%1:").arg(second, 2, 10, QChar('0'));//秒
    resultStr += QString("%1:").arg(ms, 3, 10, QChar('0'));//毫秒
    resultStr += QString::number(mms);//0.1ms

    return resultStr;
}

QString MyCanThread::analysisCanInfo(bool issend, const VCI_CAN_OBJ *info)
{
    QString show;
    QString time;
    if (info->DataLen > 0) {
        if (m_showrelativetime) {
            time = timeStampToTimeStr(info->TimeStamp);
            time.append("         ");
        } else {
            time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        }

        show.sprintf("   %s   0x%08x   %s    %s\t %d   ", 
                (issend? "发送" : "接收"), info->ID, 
                (info->ExternFlag? "扩展帧" : "标准帧"),
                (info->RemoteFlag? "远程帧" : "数据帧"), 
                info->DataLen);
        show.insert(0, time);
        
        for (int j = 0; j < info->DataLen; ++j) {
            QString str;
            str.sprintf(" %02x", info->Data[j]);
            show += str;
        }
        
        if (m_showhexupper) {
            show = show.toUpper();
        }
        show.append("\n");
    }
    return show;
}

void MyCanThread::getFrameCount(uint &recvCount, uint &sendCount)
{
    recvCount = m_recvFrameCount;
    sendCount = m_sendFrameCount;
}

void MyCanThread::setFrameCount(uint recvCount, uint sendCount)
{
    m_recvFrameCount = recvCount;
    m_sendFrameCount = sendCount;
}

void MyCanThread::getTimerStr(int index, QString &tim0, QString &tim1)
{
    if (index < 0 || index > 10) {
        return ;
    }
    
    tim0 = g_tim0[index];
    tim1 = g_tim1[index];
}

bool isInRange(uint32_t candid, const uint32_t *fromLowHex, const uint32_t *toHighHex, int len)
{
    for (int i = 0; i < len; ++i) {
        if ((candid >= fromLowHex[i]) && (candid <= toHighHex[i])) {
            return true;
        }
    }
    
    return false;
}

void MyCanThread::run()
{
    VCI_ERR_INFO errInfo;
    VCI_CAN_OBJ frameinfo[RECV_MAX_NUM];
    ULONG recvNum = 0;
    int rcvLength = 0;
    while (true) {
        usleep(THREAD_SLEEP_US);
        // ----- RX-timeout test ----------------------------------------------
        recvNum = VCI_GetReceiveNum(m_devType, m_device, m_chanel);
        if (recvNum > 0) {
            rcvLength = VCI_Receive(m_devType, m_device, m_chanel, frameinfo, RECV_MAX_NUM, 50);
            if (rcvLength < 0) { //读取失败
                VCI_ReadErrInfo(m_devType, m_device, m_chanel, &errInfo);
            } else { //解析数据
                /* 处理接收的数据 */
                QString recvinfo;
                for (int i = 0; i < rcvLength; ++i) {
                    if (1 == m_currentMode) { // 不接受IDs
                        if (isInRange(frameinfo[i].ID, m_fromLowHex, m_toHighHex, m_hexLenght)) {
                            continue;
                        }
                    } else if (2 == m_currentMode) { // 仅接收IDs
                        if (!isInRange(frameinfo[i].ID, m_fromLowHex, m_toHighHex, m_hexLenght)) {
                            continue;
                        }
                    }
#if 0
                    qWarning("rcvLength:%d, i:%d, id:0x%08x, Data:%02x %02x %02x %02x", \
                        rcvLength, i, frameinfo[i].ID, frameinfo[i].Data[0], frameinfo[i].Data[1],
                        frameinfo[i].Data[2], frameinfo[i].Data[3]);
#endif
                    m_recvFrameCount++;
                    recvinfo += analysisCanInfo(false, &frameinfo[i]);
                }
    
                if (recvinfo.length()) {
                    //qDebug() << "recvinfo:" << recvinfo;
                    if (m_throwrecvsignal) {
                        emit showInfoSignal(recvinfo);
                    }
                    
                    /* 根据接收数据 处理反馈 */
                    for (int i = 0; i < rcvLength; ++i) {
                        switch (m_protocol) {
                            case DIPRO_UDS:
                                UDS_ecmHandle(frameinfo[i].ID, frameinfo[i].Data, frameinfo[i].DataLen);
                                break;
                            case DIPRO_CCP:
                                CCP_ecmHandle(frameinfo[i].ID, frameinfo[i].Data, frameinfo[i].DataLen);
                                break;
                            case DIPRO_XCP:
                                XCP_ecmHandle(frameinfo[i].ID, frameinfo[i].Data, frameinfo[i].DataLen);
                                break;
                        }
                    }
                }
            }
        }
    }
}

void MyCanThread::UDS_ecmHandle(uint canid, const BYTE *data, BYTE datalen)
{
    int cnt = 0;
    bool sendflag = false;
    bool iswithcutline = false;
    BYTE senddata[16] = {0};
    static bool issid36data = false;
    static int delayRspCount = 0;
    static BYTE delayRspData[32] = {0};
    static BYTE preframeid = 0x20;

    if (m_reqcanid != canid || 0 == datalen) {
        return ;
    }

    if (0x30 == (data[0] & 0x30)) { // 流控帧
        return ;
    }

    if (data[0] < 0x08) {
        sendflag = true;
        iswithcutline = true;
        switch (data[1]) {
            case UDS_SESSION_CTR:
                senddata[cnt++] = 0x06;
                senddata[cnt++] = 0x50;
                senddata[cnt++] = data[2];
                senddata[cnt++] = 0x00;
                senddata[cnt++] = 0x32;
                senddata[cnt++] = 0x01;
                senddata[cnt++] = 0xea;
                break;
            case UDS_SECURITY_ACCESS:
                if (0x01 == (data[2] % 2)) {
                    senddata[cnt++] = 0x06;
                    senddata[cnt++] = 0x67;
                    senddata[cnt++] = data[2];
                    senddata[cnt++] = 0x1a;
                    senddata[cnt++] = 0x2b;
                    senddata[cnt++] = 0x3c;
                    senddata[cnt++] = 0x4d;
                } else {
                    senddata[cnt++] = 0x02;
                    senddata[cnt++] = 0x67;
                    senddata[cnt++] = data[2];
                }
                break;
            case UDS_ROUTINE_CTR:
                if (0xFA == data[2] && 0xFF == data[3]) {
                    senddata[cnt++] = data[0];
                    senddata[cnt++] = 0x71;
                    senddata[cnt++] = data[2];
                    senddata[cnt++] = 0x00;
                } else if ((0x02 == data[3] && 0x02 == data[4]) 
                    || (0xFF == data[3] && 0x00 == data[4])) {
                    senddata[cnt++] = data[0] - 1;
                    senddata[cnt++] = 0x71;
                    senddata[cnt++] = data[2];
                    senddata[cnt++] = data[3];
                    senddata[cnt++] = data[4];
                    senddata[cnt++] = 0x00;
                } else {
                    senddata[cnt++] = data[0] + 1;
                    senddata[cnt++] = 0x71;
                    for (int i = 0; i < data[0] - 1; ++i) {
                        senddata[cnt++] = data[2 + i];
                    }
                    senddata[cnt++] = 0x00;
                }
                break;
            case UDS_REQ_TRANSFER_EXIT:
                senddata[cnt++] = 0x01;
                senddata[cnt++] = 0x77;
                break;
            case UDS_WRITE_FINGER_PRINT:
                senddata[cnt++] = 0x02;
                senddata[cnt++] = 0x7B;
                senddata[cnt++] = data[2];
                break;
            default:
                if (UDS_TRANSFER_DATA == data[1]) {
                    if (m_genaratepacket && m_file.isOpen()) {
                        if (data[0] > 2) {
                            transferOtaData(&data[3], data[0] - 2);
                        }
                    }
                } else if (UDS_ECU_RESET == data[1]) {
                    m_file.close();
                }
                memcpy(senddata, data, datalen);
                senddata[1] += 0x40;
                break;
        }
    } else if (0x10 == (data[0] & 0x10)) {
        preframeid = 0x20;
        senddata[0] = 0x30; // 先回复流控帧
        senddata[2] = m_streaminterval; // 两帧间隔5ms
        sendflag = m_streamcontrol;
        delayRspCount = (((data[0] & 0x0f) << 8) + data[1]) - 6;
        //qDebug("1delayRspCount:%d, data[0]:%02x, data[1]:%02x data[2]:%02x, data[3]:%02x", 
        //    delayRspCount, data[0], data[1], data[2], data[3]);
        switch (data[2]) {
            case UDS_ROUTINE_CTR:
                delayRspData[cnt++] = 0x05;
                delayRspData[cnt++] = 0x71;
                delayRspData[cnt++] = data[3];
                delayRspData[cnt++] = data[4];
                delayRspData[cnt++] = data[5];
                delayRspData[cnt++] = 0x00;
                break;
            case UDS_REQ_DOWNLOAD:
                delayRspData[cnt++] = 0x04;
                delayRspData[cnt++] = 0x74;
                delayRspData[cnt++] = 0x20;
                delayRspData[cnt++] = 0x0f;
                delayRspData[cnt++] = 0xff;
                break;
            case UDS_WRITE_FINGER_PRINT:
                delayRspData[cnt++] = 0x02;
                delayRspData[cnt++] = 0x7B;
                delayRspData[cnt++] = data[3];
                break;
            case UDS_SECURITY_ACCESS:
                delayRspData[cnt++] = 0x02;
                delayRspData[cnt++] = 0x67;
                delayRspData[cnt++] = data[3];
                break;
            case UDS_TRANSFER_DATA:
                delayRspData[cnt++] = 0x02;
                delayRspData[cnt++] = 0x76;
                delayRspData[cnt++] = data[3];
                if (m_genaratepacket && m_file.isOpen()) {
                    transferOtaData(&data[4], 4);
                    issid36data = true;
                }
                break;
            default:
                delayRspData[cnt++] = 0x02;
                delayRspData[cnt++] = data[2] + 0x40;
                delayRspData[cnt++] = data[3];
                break;
        } 
        //if (0x34 == data[2] || 0x31 == data[2]) {
        //    memcpy(senddata, delayRspData, 8);
        //    iswithcutline = true;
        //}
    } else if (0x20 == (data[0] & 0x20)) {
        if ((preframeid + 0x01) == data[0]) {
            preframeid = data[0];
            if (0x2f == preframeid) {
                preframeid = 0x1f;
            }
        } else {
            preframeid = data[0];
#if IS_ENABEL_RECV_ERROR_MONITOR > 0
            QString extend;
            extend.sprintf("帧序号出错-%08x: %02x %02x %02x %02x %02x %02x %02x %02x", 
                    canid, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
            emit errorInfoSignal(ERROR_RECV, extend);

            VCI_ClearBuffer(m_devType, m_device, m_chanel);
            exit(-1);
#endif
        }
        if (delayRspData[0]) {
            if (m_genaratepacket && m_file.isOpen() && issid36data) {
                transferOtaData(&data[1], (delayRspCount > 7)? 7 : delayRspCount);
            }
            delayRspCount -= 7;
            //qDebug("2delayRspCount:%d, data[1]:%02x, data[2]:%02x", delayRspCount, data[1], data[2]);
            if (delayRspCount <= 0) {
                memcpy(senddata, delayRspData, 8);
                memset(delayRspData, 0, sizeof(delayRspData));
                sendflag = true;
                iswithcutline = true;
                issid36data = false;
            }
        }
    }

    if (sendflag && m_autoresponse) {
        for (int i = senddata[0]; (i > 0 && i < 8); ++i) {
            senddata[1 + i] = 0xFF;
        }
        sendCanData(iswithcutline, m_rspcanid, senddata, 8);
    }
}

void MyCanThread::CCP_ecmHandle(uint canid, const BYTE *data, BYTE datalen)
{
    int cnt = 0;
    bool iswithcutline = false;
    BYTE senddata[16] = {0};
    static BYTE precmd = 0;
    static uint32_t mta0 = 0;
    static uint8_t extaddr = 0;

    if (m_reqcanid != canid || 0 == datalen) {
        return ;
    }

    senddata[cnt++] = 0xFF;     // CRM
    senddata[cnt++] = 0x00;     // ERR
    senddata[cnt++] = data[1];  // CTR
    switch (data[0]) {
        case CCP_EXCHANGE_ID:
            senddata[cnt++] = 0x07; // ID字节长度
            senddata[cnt++] = 0x00; // ID数据类型
            senddata[cnt++] = m_ccpresuse; // 资源可用性
            senddata[cnt++] = m_ccpressave; // 资源保护性
            break;
        case CCP_BUILD_CHECKSUM:
            senddata[cnt++] = 0x02; // 校验和字节数
            senddata[cnt++] = 0x2d; // 校验和
            senddata[cnt++] = 0xc5;
            break;
       case CCP_SET_MTA:
            extaddr = data[3];
            mta0 = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | (data[7] << 0);
            break;
       case CCP_DNLOAD:
            if (m_genaratepacket && m_file.isOpen()) {
                transferOtaData(&data[3], data[2]);
            }
            mta0 += data[2];
            senddata[cnt++] = extaddr;
            senddata[cnt++] = (uint8_t)(mta0 >> 24); // 校验和字节数
            senddata[cnt++] = (uint8_t)(mta0 >> 16); // 校验和
            senddata[cnt++] = (uint8_t)(mta0 >> 8);
            senddata[cnt++] = (uint8_t)(mta0 >> 0);
            break;
       case CCP_DNLOAD_6:
            if (m_genaratepacket && m_file.isOpen()) {
               transferOtaData(&data[2], 6);
            }
            mta0 += 6;
            senddata[cnt++] = extaddr;
            senddata[cnt++] = (uint8_t)(mta0 >> 24); // 校验和字节数
            senddata[cnt++] = (uint8_t)(mta0 >> 16); // 校验和
            senddata[cnt++] = (uint8_t)(mta0 >> 8);
            senddata[cnt++] = (uint8_t)(mta0 >> 0);
            break;
        case CCP_DISCONNECT:
            m_file.close();
            break;
        default:
            break;
    }
    
    if (precmd != data[0]) {
        precmd = data[0];
        iswithcutline = true;
    } else {
        iswithcutline = false;
    }
    
    memcpy(senddata + cnt, &data[2], 8 - cnt);
    if (m_autoresponse) {
        sendCanData(iswithcutline, m_rspcanid, senddata, 8);
    }
}

void MyCanThread::XCP_ecmHandle(uint canid, const BYTE *data, BYTE datalen)
{
    int cnt = 0;
    bool iswithcutline = false;
    BYTE senddata[16] = {0};
    static BYTE precmd = 0;
    static uint32_t mta0 = 0;
    static uint8_t extaddr = 0;

    if (m_reqcanid != canid || 0 == datalen) {
        return ;
    }

    senddata[cnt++] = 0xFF;     // CRM
    senddata[cnt++] = 0x00;     // ERR
    senddata[cnt++] = data[1];  // CTR
    switch (data[0]) {
        case CCP_EXCHANGE_ID:
            senddata[cnt++] = 0x07; // ID字节长度
            senddata[cnt++] = 0x00; // ID数据类型
            senddata[cnt++] = m_ccpresuse; // 资源可用性
            senddata[cnt++] = m_ccpressave; // 资源保护性
            break;
        case CCP_BUILD_CHECKSUM:
            senddata[cnt++] = 0x02; // 校验和字节数
            senddata[cnt++] = 0x2d; // 校验和
            senddata[cnt++] = 0xc5;
            break;
       case CCP_SET_MTA:
            extaddr = data[3];
            mta0 = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | (data[7] << 0);
            break;
       case CCP_DNLOAD:
           mta0 += data[2];
           senddata[cnt++] = extaddr;
           senddata[cnt++] = (uint8_t)(mta0 >> 24); // 校验和字节数
           senddata[cnt++] = (uint8_t)(mta0 >> 16); // 校验和
           senddata[cnt++] = (uint8_t)(mta0 >> 8);
           senddata[cnt++] = (uint8_t)(mta0 >> 0);
           break;
       case CCP_DNLOAD_6:
           mta0 += 6;
           senddata[cnt++] = extaddr;
           senddata[cnt++] = (uint8_t)(mta0 >> 24); // 校验和字节数
           senddata[cnt++] = (uint8_t)(mta0 >> 16); // 校验和
           senddata[cnt++] = (uint8_t)(mta0 >> 8);
           senddata[cnt++] = (uint8_t)(mta0 >> 0);
           break;
        default:
            break;
    }
    
    if (precmd != data[0]) {
        precmd = data[0];
        iswithcutline = true;
    } else {
        iswithcutline = false;
    }
    
    memcpy(senddata + cnt, &data[2], 8 - cnt);
    if (m_autoresponse) {
        sendCanData(iswithcutline, m_rspcanid, senddata, 8);
    }
}

void MyCanThread::sendCanData(bool iswithcutline, uint canid, const BYTE *data, BYTE datalen)
{
    VCI_CAN_OBJ cansend;
    
    memset(&cansend, 0, sizeof(cansend));
    cansend.SendType = m_sendType; /* 0：正常发送 1：单次发送 2：自发自收 3：单次自发自收 */
    cansend.ID = canid;
    cansend.DataLen = datalen;
    cansend.RemoteFlag = m_remoteFlag;
    cansend.ExternFlag = (canid > 0xFFFF)? 1 : m_externFlag;
    cansend.TimeStamp += 2;
    memcpy(cansend.Data, data, datalen);

    //qDebug("SendType:%d, ID:0x%08x, m_remoteFlag:%d, m_externFlag:%d, datalen:%d", 
    //    m_sendType, canid, m_remoteFlag, m_externFlag, datalen);
    if (1 == VCI_Transmit(m_devType, m_device, m_chanel, &cansend, 1)) {
        m_sendFrameCount++;
        QString sendinfo = analysisCanInfo(true, &cansend);
        if (m_withcutline && iswithcutline) {
            sendinfo.append("----------------------------------------"
                "-----------------------------------------------------\n");
        }
        emit showInfoSignal(sendinfo);
    } else {
        QString extend;
        extend.sprintf("%08x: %02x %02x %02x %02x %02x %02x %02x %02x", 
                canid, cansend.Data[0], cansend.Data[1], cansend.Data[2], cansend.Data[3], 
                cansend.Data[4], cansend.Data[5], cansend.Data[6], cansend.Data[7]);
        emit errorInfoSignal(ERROR_SEND, extend);
    }
}

void MyCanThread::startupAutoDiagI(uint32_t startCANID, const BYTE *data, uint8_t datalen)
{
    if (m_genaratepacket) {
        if (m_file.isOpen()) {
            m_file.close();
        }
        QString prefix;
        switch (m_protocol) {
            case DIPRO_UDS: prefix.append("UDS"); break;
            case DIPRO_CCP: prefix.append("CCP"); break;
            case DIPRO_XCP: prefix.append("XCP"); break;
            case DIPRO_UNKNOWN: default: prefix.append("UNKNOWN"); break;
        }
        QString curtime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss");
        //m_file.setFileName(tr("./Record/%1-TRANS-PACKET-%2.bin").arg(prefix, curtime));
        //m_file.open(QIODevice::WriteOnly | QIODevice::Truncate);
        m_file.setFileName(tr("./Record/%1-TRANS-PACKET-123456.bin").arg(prefix));
        m_file.open(QIODevice::WriteOnly | QIODevice::Append);
    }

    sendCanData(false, startCANID, data, datalen);
}

void MyCanThread::setFilterParam(int currentMode, const uint32_t *fromLowHex, 
                            const uint32_t *toHighHex, int len)
{
    m_currentMode = currentMode;
    m_hexLenght = len;
    
    //qDebug() << "m_currentMode:" << m_currentMode << "m_hexLenght:" << m_hexLenght;
    for (int i = 0; i < m_hexLenght; ++i) {
        m_fromLowHex[i] = fromLowHex[i];
        m_toHighHex[i] = toHighHex[i];
        //qDebug("LowHex:0x%08x, HighHex:0x%08x", m_fromLowHex[i], m_toHighHex[i]);
    }
}

void MyCanThread::slotStreamControl(int value)
{
    m_streamcontrol = (0 != value);
}

void MyCanThread::slotGenaratePacket(int value)
{
    if (!value && m_file.isOpen()) {
        m_file.close();
    }
    m_genaratepacket = (0 != value);
}

void MyCanThread::slotWithCutline(int value)
{
    m_withcutline = (0 != value);
}

void MyCanThread::slotStreamInterval(const QString &value)
{
    m_streaminterval = value.toShort() & 0x00FF;
}

void MyCanThread::slotCcpResource(int resUse, int resSave)
{
    m_ccpresuse = (uint8_t)resUse;
    m_ccpressave = (uint8_t)resSave;
}

void MyCanThread::setProtocol(int protocol)
{
    m_protocol = protocol;
}

void MyCanThread::setAutoResponse(int response)
{
    m_autoresponse = response;
}

