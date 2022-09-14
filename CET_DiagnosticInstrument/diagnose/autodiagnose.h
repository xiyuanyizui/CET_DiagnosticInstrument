#ifndef AUTODIAGNOSE_H
#define AUTODIAGNOSE_H

#include "mycanthread.h"

#include <QDialog>

namespace Ui {
class AutoDiagnose;
}

class AutoDiagnose : public QDialog
{
    Q_OBJECT

public:
    explicit AutoDiagnose(QThread *thread, QWidget *parent = 0);
    ~AutoDiagnose();
    bool getStartCanid(int protocol, uint32_t *startcanid, BYTE *data, uint8_t *datalen);
    bool getCommCanid(int protocol, uint32_t *reqcanid, uint32_t *rspcanid);

public slots:
    void slotCcpResourceHande();
signals:
    void signalCcpResource(int resUse, int resSave);
private:
    Ui::AutoDiagnose *ui;

    int m_protocol;
};

#endif // AUTODIAGNOSE_H
