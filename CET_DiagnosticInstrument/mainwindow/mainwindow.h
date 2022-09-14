#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "mycanthread.h"
#include "filterdialog.h"
#include "cetlicenseinterface.h"
#include "cetupdateinterface.h"
#include "autodiagnose.h"

#include <QMainWindow>
#include <QLabel>
#include <QMap>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
	void initDllPlugin();
    void keyPressEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent * event);

public slots:
    void slotActivateDialog();
    void slotStyleActions(QAction *action);
    void slotControlDevice();
    void slotSaveLog();
    void slotClearLog();
    void slotClearCountButton();
    void slotRadioButton();
    void slotControlAutoDiagnose();
    void slotShowLog(const QString &info);
    void slotBaudrateIndex(int index);
    void slotTimeComboBox(int index);
    void slotCaseHexComboBox(int index);
    void slotErrorInfo(int error, const QString &extend);
    void slotFilterDialog();
    void slotProtocolSelect(int index);

private:
    QObject *loadDllPlugin(const QString &dllname);

private:
    QMap<QString, int> m_mapDevtype;
    QLabel *sendFrameLabel;
    QLabel *recvFrameLabel;
    MyCanThread *canThread;
    FilterDialog *filterDlg;
    CetLicenseInterface *m_cetLicIface;
	CetUpdateInterface *m_cetUpIface;
    AutoDiagnose *autoDiagDlg;
    
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
