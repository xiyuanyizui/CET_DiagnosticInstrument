#include "autodiagnose.h"
#include "ui_autodiagnose.h"

#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QDebug>

AutoDiagnose::AutoDiagnose(QThread *thread, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AutoDiagnose)
{
    ui->setupUi(this);
    setWindowTitle(tr("下线检测诊断仪 - 自动诊断配置"));

    connect(ui->CALUseCheckBox, &QCheckBox::clicked, ui->CALSaveCheckBox, &QCheckBox::setEnabled);
    connect(ui->PGMUseCheckBox, &QCheckBox::clicked, ui->PGMSaveCheckBox, &QCheckBox::setEnabled);
    connect(ui->DAQUseCheckBox, &QCheckBox::clicked, ui->DAQSaveCheckBox, &QCheckBox::setEnabled);

    connect(ui->CALUseCheckBox, &QCheckBox::clicked, this, &AutoDiagnose::slotCcpResourceHande);
    connect(ui->PGMUseCheckBox, &QCheckBox::clicked, this, &AutoDiagnose::slotCcpResourceHande);
    connect(ui->DAQUseCheckBox, &QCheckBox::clicked, this, &AutoDiagnose::slotCcpResourceHande);
    connect(ui->CALSaveCheckBox, &QCheckBox::clicked, this, &AutoDiagnose::slotCcpResourceHande);
    connect(ui->PGMSaveCheckBox, &QCheckBox::clicked, this, &AutoDiagnose::slotCcpResourceHande);
    connect(ui->DAQSaveCheckBox, &QCheckBox::clicked, this, &AutoDiagnose::slotCcpResourceHande);
    

    MyCanThread *canThread = static_cast<MyCanThread *>(thread);
    if (canThread) {
        connect(ui->streamControlCheckBox, &QCheckBox::stateChanged, canThread, &MyCanThread::slotStreamControl);
        connect(ui->streamIntervalLineEdit, &QLineEdit::textChanged, canThread, &MyCanThread::slotStreamInterval);
        connect(this, &AutoDiagnose::signalCcpResource, canThread, &MyCanThread::slotCcpResource);

        canThread->slotStreamControl(ui->streamControlCheckBox->checkState());
        canThread->slotStreamInterval(ui->streamIntervalLineEdit->text());

        slotCcpResourceHande();
    }
}

AutoDiagnose::~AutoDiagnose()
{
    delete ui;
}

bool AutoDiagnose::getStartCanid(int protocol, uint32_t *startcanid, BYTE *data, uint8_t *datalen)
{
    bool ok;
    QString startCANID;
    QString startData;
    bool result = true;
    
    switch (protocol) {
        case DIPRO_UDS:
            startCANID = ui->startCANIDLineEdit_UDS->text();
            *startcanid = (uint32_t)startCANID.toUInt(&ok, 16);
        
            startData = ui->startDataLineEdit_UDS->text();
            startData.replace(" ", "");
            *datalen = 0;
            for (int i = 0; i < startData.length(); i += 2) {
                data[(*datalen)++] = startData.mid(i, 2).toUShort(&ok, 16);
            }
            break;
        case DIPRO_CCP:
            startCANID = ui->startCANIDLineEdit_CCP->text();
            *startcanid = (uint32_t)startCANID.toUInt(&ok, 16);
            
            startData = ui->startDataLineEdit_CCP->text();
            startData.replace(" ", "");
            *datalen = 0;
            for (int i = 0; i < startData.length(); i += 2) {
                data[(*datalen)++] = startData.mid(i, 2).toUShort(&ok, 16);
            }
            break;
        case DIPRO_XCP:
            startCANID = ui->startCANIDLineEdit_XCP->text();
            *startcanid = (uint32_t)startCANID.toUInt(&ok, 16);
            
            startData = ui->startDataLineEdit_XCP->text();
            startData.replace(" ", "");
            *datalen = 0;
            for (int i = 0; i < startData.length(); i += 2) {
                data[(*datalen)++] = startData.mid(i, 2).toUShort(&ok, 16);
            }
            break;
        case DIPRO_UNKNOWN:
            result = false;
        default:
            break;
    }

    return result;
}

bool AutoDiagnose::getCommCanid(int protocol, uint32_t *reqcanid, uint32_t *rspcanid)
{
    bool ok;
    QString reqCANID;
    QString rspCANID;
    bool result = true;

    switch (protocol) {
        case DIPRO_UDS:
            reqCANID = ui->reqCANIDLineEdit_UDS->text();
            *reqcanid = (uint32_t)reqCANID.toUInt(&ok, 16);
            rspCANID = ui->rspCANIDLineEdit_UDS->text();
            *rspcanid = (uint32_t)rspCANID.toUInt(&ok, 16);
            break;
        case DIPRO_CCP:
            reqCANID = ui->reqCANIDLineEdit_CCP->text();
            *reqcanid = (uint32_t)reqCANID.toUInt(&ok, 16);
            rspCANID = ui->rspCANIDLineEdit_CCP->text();
            *rspcanid = (uint32_t)rspCANID.toUInt(&ok, 16);
            break;
        case DIPRO_XCP:
            reqCANID = ui->reqCANIDLineEdit_XCP->text();
            *reqcanid = (uint32_t)reqCANID.toUInt(&ok, 16);
            rspCANID = ui->rspCANIDLineEdit_XCP->text();
            *rspcanid = (uint32_t)rspCANID.toUInt(&ok, 16);
            break;
        case DIPRO_UNKNOWN:
            result = false;
        default:
            break;
    }

    return result;
}

void AutoDiagnose::slotCcpResourceHande()
{
    int resUse = 0, resSave = 0;

    resUse |= ui->CALUseCheckBox->isChecked()? (1 << 0) : 0;
    resUse |= ui->PGMUseCheckBox->isChecked()? (1 << 6) : 0;
    resUse |= ui->DAQUseCheckBox->isChecked()? (1 << 1) : 0;

    resSave |= ui->CALSaveCheckBox->isChecked()? (1 << 0) : 0;
    resSave |= ui->PGMSaveCheckBox->isChecked()? (1 << 6) : 0;
    resSave |= ui->DAQSaveCheckBox->isChecked()? (1 << 1) : 0;

    emit signalCcpResource(resUse, resSave);
}

