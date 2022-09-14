#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStyleFactory>
#include <QListWidget>
#include <QDateTime>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QTextBlock>
#include <QDesktopServices>
#include <QPluginLoader>

#include "other/version.h"

#include <QDebug>

#define RECORD_DIR "Record"

#if (IS_RELEASE_VERSION > 0)
#define VERSION_TIP "正式版"
#else
#define VERSION_TIP "测试版"
#endif

#define CETDIAGNOSTICINSTRUMENT_VERSION "CET下线检测诊断仪 V" PRODUCT_VERSION_STR " " VERSION_TIP
#define CETDIAGNOSTICINSTRUMENT_SETUP 	"CET_DiagnosticInstrument-Setup"

#define DATA_LOG_DOCUMENT_MAX_ROWS 10000000

#define SEND_FRAME_NUM "  发送帧数："
#define RECV_FRAME_NUM "  接收帧数："

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_cetLicIface(nullptr),
    m_cetUpIface(nullptr)
{    
    ui->setupUi(this);
    this->setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    this->setWindowTitle(tr(CETDIAGNOSTICINSTRUMENT_VERSION));
    this->setFixedSize(sizeHint());
    
    m_mapDevtype["VCI_USBCAN1(I+)"] = VCI_USBCAN1;
    m_mapDevtype["VCI_USBCAN2(II+)"]= VCI_USBCAN2;
    m_mapDevtype["VCI_USBCAN2A"]    = VCI_USBCAN2A;
    m_mapDevtype["VCI_USBCAN_E_U"]  = VCI_USBCAN_E_U;
    m_mapDevtype["VCI_USBCAN_2E_U"] = VCI_USBCAN_2E_U;
    for(QMap<QString, int>::ConstIterator ite = m_mapDevtype.constBegin(); 
        ite != m_mapDevtype.constEnd(); ++ite) {
        ui->typeComboBox->addItem(ite.key());
    }

    sendFrameLabel = new QLabel(tr("%1%2").arg(SEND_FRAME_NUM).arg(0));
    recvFrameLabel = new QLabel(tr("%1%2").arg(RECV_FRAME_NUM).arg(0));
    QPushButton *clearCountButton = new QPushButton(tr("清空计数"), this);
    QStatusBar *sBar = statusBar();
    sBar->addWidget(sendFrameLabel, 1);
    sBar->addWidget(recvFrameLabel, 1);
    sBar->addWidget(clearCountButton);

    /** 获取支持的系统风格 
        string: "Windows"
        string: "WindowsXP"
        string: "WindowsVista"
        string: "Fusion" */
    QAction *action = NULL;
    QStringList styleList = QStyleFactory::keys();
    QActionGroup *styleActions = new QActionGroup(this);
    for (QString stylename : styleList) {
        action = styleActions->addAction(stylename);
        action->setCheckable(true);
        action->setChecked(true);
    }

    slotStyleActions(action);
    
    ui->styleMenu->addActions(styleActions->actions());
    ui->centralWidget->layout()->setMargin(15);
    ui->centralWidget->layout()->setSpacing(8);

    filterDlg = new FilterDialog(this);
    canThread = new MyCanThread();
    autoDiagDlg = new AutoDiagnose(canThread, this);
    
    QDir dir;
    QString opendir = tr("%1/%2").arg(QDir::currentPath(), RECORD_DIR);
    if (!dir.exists(opendir)) {
        dir.mkdir(opendir);
    }

    ui->timer0LineEdit->setEnabled(false);
    ui->timer1LineEdit->setEnabled(false);
    //ui->dataLogTextBrowser->setFocus();
    ui->dataLogTextBrowser->setReadOnly(true);
    QTextDocument *textDocument = ui->dataLogTextBrowser->document();
    textDocument->setMaximumBlockCount(DATA_LOG_DOCUMENT_MAX_ROWS); 

    connect(ui->activateAction, &QAction::triggered, this, &MainWindow::slotActivateDialog);
    connect(ui->configToolButton, &QToolButton::clicked, autoDiagDlg, &AutoDiagnose::show);

    connect(styleActions, &QActionGroup::triggered, this, &MainWindow::slotStyleActions);
    connect(clearCountButton, &QPushButton::clicked, this, &MainWindow::slotClearCountButton);

    connect(ui->autoRadioButton, &QRadioButton::clicked, this, &MainWindow::slotRadioButton);
    connect(ui->pointTriggerRadioButton, &QRadioButton::clicked, this, &MainWindow::slotRadioButton);

    connect(ui->connectButton, &QPushButton::clicked, this, &MainWindow::slotControlDevice);
    connect(ui->saveLogButton, &QPushButton::clicked, this, &MainWindow::slotSaveLog);
    connect(ui->clearButton, &QPushButton::clicked, this, &MainWindow::slotClearLog);
    connect(ui->DIAutoButton, &QPushButton::clicked, this, &MainWindow::slotControlAutoDiagnose);
    connect(ui->DIPointTriggerButton, &QPushButton::clicked, this, &MainWindow::slotControlAutoDiagnose);
    connect(ui->filterButton, &QPushButton::clicked, this, &MainWindow::slotFilterDialog);
    connect(ui->protocolSelectComboBox, SIGNAL(activated(int)), this, SLOT(slotProtocolSelect(int)));
    connect(ui->timeComboBox, SIGNAL(activated(int)), this, SLOT(slotTimeComboBox(int)));
    connect(ui->caseHexComboBox, SIGNAL(activated(int)), this, SLOT(slotCaseHexComboBox(int)));

    connect(canThread, &MyCanThread::showInfoSignal, this, &MainWindow::slotShowLog);
    connect(canThread, &MyCanThread::errorInfoSignal, this, &MainWindow::slotErrorInfo);
    connect(ui->baudrateComboBox, SIGNAL(activated(int)), this, SLOT(slotBaudrateIndex(int)));

    connect(ui->withCutlineCheckBox, &QCheckBox::stateChanged, canThread, &MyCanThread::slotWithCutline);
    connect(ui->genaratePacketCheckBox, &QCheckBox::stateChanged, canThread, &MyCanThread::slotGenaratePacket);

    ui->licenseMenu->menuAction()->setVisible(false);

    canThread->slotWithCutline(ui->withCutlineCheckBox->checkState());

    slotProtocolSelect(ui->protocolSelectComboBox->currentIndex());

    initDllPlugin();
}


MainWindow::~MainWindow()
{
    delete sendFrameLabel;
    delete recvFrameLabel;
    delete canThread;
    delete filterDlg;
    delete m_cetLicIface;
    delete m_cetUpIface;
    delete autoDiagDlg;
    delete ui;
}

QObject *MainWindow::loadDllPlugin(const QString &dllname)
{
	QObject *plugin = nullptr;
    QDir pluginsDir("./plugins");

    foreach (const QString &fileName, pluginsDir.entryList(QDir::Files)) {
        //qDebug() << "fileName" << fileName << "absoluteFilePath(fileName)" << pluginsDir.absoluteFilePath(fileName);
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        plugin = pluginLoader.instance();
        if (plugin) {
            plugin->setParent(this);
            if (0 == dllname.compare(fileName, Qt::CaseInsensitive)) {
                break;
            }
        }
    }

    return plugin;
}

void MainWindow::initDllPlugin()
{
    QFileInfo fileInfo("./change.log");
    if (fileInfo.size() < 64) {
        QFile file("./change.log");
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QString details;
        details.append("/***********************************************************************************************\n");
        details.append(tr("* 项目名称：%1\n").arg(QString(CETDIAGNOSTICINSTRUMENT_SETUP).split('-').first()));
        details.append("* 项目简介：CAN下线诊断应用程序\n");
        details.append("* 创建日期：2022.05.11\n");
        details.append("* 创建人员：郑建宇(CetXiyuan)\n");
        details.append("* 版权说明：本程序仅为个人兴趣开发，有需要者可以免费使用，但是请不要在网络上进行恶意传播\n");
        details.append("***********************************************************************************************/\n\n");

        file.seek(0);
        file.write(details.toUtf8());
        file.close();
    }

    m_cetLicIface = qobject_cast<CetLicenseInterface *>(loadDllPlugin("CetLicensePlugin.dll"));
    if (!m_cetLicIface) {
        QMessageBox::warning(this, tr("警告"), tr("缺少 CetLicensePlugin.dll 库\t"));
    }

    m_cetUpIface = qobject_cast<CetUpdateInterface *>(loadDllPlugin("CetUpdatePlugin.dll"));
    if (!m_cetUpIface) {
        QMessageBox::warning(this, tr("警告"), tr("缺少 CetUpdatePlugin.dll 库\t"));
    }

    
    if (!m_cetLicIface || CetLicenseInterface::ACTIVATE_OK != m_cetLicIface->activate(PRODUCT_NAME)) {
        this->centralWidget()->setEnabled(false);
        this->setWindowTitle(windowTitle().append(tr(" [已到期]")));
    }

    if (m_cetUpIface) {
        m_cetUpIface->checkUpdate(CETDIAGNOSTICINSTRUMENT_SETUP, PRODUCT_VERSION_STR, QCoreApplication::quit);
    }
}


void MainWindow::slotActivateDialog()
{
    if (!m_cetLicIface)
        return;
    
    m_cetLicIface->exec();
    int result = m_cetLicIface->result();
    if (CetLicenseInterface::ACTIVATE_OK == result) {
        this->centralWidget()->setEnabled(true);
        this->setWindowTitle(tr(CETDIAGNOSTICINSTRUMENT_VERSION));
    }

    if (CetLicenseInterface::ACTIVATE_NONE != result) {
        QMessageBox::information(this, tr("通知"), m_cetLicIface->message() + "\t", QMessageBox::Ok);
    }
}


void MainWindow::slotStyleActions(QAction *action)
{
    QApplication::setStyle(QStyleFactory::create(action->text()));
}

void MainWindow::slotControlDevice()
{
    QString strtime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    
    if (tr("连接") == ui->connectButton->text()) {
        QString accessCode = ui->accessCodeLineEdit->text();
        QString maskCode = ui->maskCodeLineEdit->text();
        if (8 != accessCode.length()) {
            QMessageBox::warning(this, tr("warning"), tr("验收码数据格式不对 !"), QMessageBox::Ok);
            return ;
        }
        if (8 != maskCode.length()) {
            QMessageBox::warning(this, tr("warning"), tr("屏蔽码数据格式不对 !"), QMessageBox::Ok);
            return ;
        }
        canThread->setDeviceInfo(m_mapDevtype.value(ui->typeComboBox->currentText()),
                                ui->indexNumberComboBox->currentIndex(),
                                ui->CANIndexComboBox->currentIndex());
        if (!canThread->controlCanDevice(true)) {
            ui->connectButton->setChecked(false);
            return ;
        }
        
        bool ok = false;
        canThread->setDeviceParam(accessCode.toUInt(&ok, 16), maskCode.toUInt(&ok, 16),
                                ui->baudrateComboBox->currentIndex(),
                                ui->sendTypeComboBox->currentIndex(),
                                ui->frameFormatComboBox->currentIndex(),
                                ui->frameTypeComboBox->currentIndex());
        if (!canThread->startupCanDevice()) {
            return ;
        }
        canThread->setThrowRecvSignal(true);
        canThread->start(QThread::HighestPriority);
                                
        ui->devParamGroupBox->setEnabled(false);
        ui->initCANParamGroupBox->setEnabled(false);
        ui->connectButton->setText(tr("断开"));

        ui->dataLogTextBrowser->append(tr("%1 打开设备成功!").arg(strtime));
        ui->dataLogTextBrowser->append(tr("%1 启动设备\n").arg(strtime));
    } else {
        canThread->controlCanDevice(false);
        canThread->quit();
        
        ui->devParamGroupBox->setEnabled(true);
        ui->initCANParamGroupBox->setEnabled(true);
        ui->connectButton->setText(tr("连接"));

        ui->dataLogTextBrowser->append(tr("%1 断开设备成功!\n").arg(strtime));
    }
}

void MainWindow::slotSaveLog()
{
    QString prefix;
    switch (ui->protocolSelectComboBox->currentIndex()) {
        case DIPRO_UDS: prefix.append("UDS"); break;
        case DIPRO_CCP: prefix.append("CCP"); break;
        case DIPRO_XCP: prefix.append("XCP"); break;
        case DIPRO_UNKNOWN: default: prefix.append("UNKNOWN"); break;
    }
    QString curtime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss");
    QString filePath = QFileDialog::getSaveFileName(this, tr("另存为"), 
        tr("./%1/%2-LOG-%3.txt").arg(RECORD_DIR, prefix, curtime), 
        "文本文件 (*.txt);;所有文件 (*.*)");
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QMessageBox::critical(this, tr("error"), tr("%1 文件保存失败 !").arg(filePath), QMessageBox::Ok);
            return ;
        }

        QTextStream stream(&file);
        /* ui->dataLogTextEdit->toPlainText()直接转字符串 因文件太大 
           会造成动态内存分配失败 所以采用下列这种方式保存 */
        QTextBlock block = ui->dataLogTextBrowser->document()->begin();
        while (block.isValid()) {
            stream << block.text().toUtf8() << "\n";
            stream.flush();
            block = block.next();
        }
        file.close();
    }
}

void MainWindow::slotClearLog()
{
    ui->dataLogTextBrowser->clear();
    slotClearCountButton();
}

void MainWindow::slotClearCountButton()
{
    canThread->setFrameCount(0, 0);
    sendFrameLabel->setText(tr("%1%2").arg(SEND_FRAME_NUM).arg(0));
    recvFrameLabel->setText(tr("%1%2").arg(RECV_FRAME_NUM).arg(0));
}

void MainWindow::slotShowLog(const QString &info)
{
    uint recvFrameCount = 0;
    uint sendFrameCount = 0;
    canThread->getFrameCount(recvFrameCount, sendFrameCount);
    sendFrameLabel->setText(tr("%1%2").arg(SEND_FRAME_NUM).arg(sendFrameCount));
    recvFrameLabel->setText(tr("%1%2").arg(RECV_FRAME_NUM).arg(recvFrameCount));

    if ((recvFrameCount + sendFrameCount) >= DATA_LOG_DOCUMENT_MAX_ROWS) {
        slotClearLog();
    }

    //qDebug() << "info:" << info;
    ui->dataLogTextBrowser->insertPlainText(info);
    ui->dataLogTextBrowser->moveCursor(QTextCursor::End);
}

void MainWindow::slotRadioButton()
{
    QRadioButton *button = static_cast<QRadioButton *>(sender());
    if (!button) {
        return;
    }
    
    if (ui->autoRadioButton == button) {
        canThread->setAutoResponse(true);
    } else if (ui->pointTriggerRadioButton == button) {
        canThread->setAutoResponse(false);
    }
}

void MainWindow::slotControlAutoDiagnose()
{
    QPushButton *button = static_cast<QPushButton *>(sender());
    if (!button) {
        return;
    }
    
    if (tr("连接") == ui->connectButton->text()) {
        QMessageBox::warning(this, tr("warning"), tr("请先连接设备 !"), QMessageBox::Ok);
        return ;
    }

    uint8_t datalen = 0;
    uint32_t startcanid;
    BYTE data[16] = {0};

    int protocol = ui->protocolSelectComboBox->currentIndex();
    if (!autoDiagDlg->getStartCanid(protocol, &startcanid, data, &datalen)) {
        QMessageBox::warning(this, tr("warning"), tr("没有配置启动CANID !"));
        return ;
    }
    
    if (ui->DIAutoButton == button) {
        canThread->setAutoResponse(true);
        ui->autoRadioButton->setChecked(true);
    } else if (ui->DIPointTriggerButton == button) {
        canThread->setAutoResponse(false);
        ui->pointTriggerRadioButton->setChecked(true);
    }

    canThread->startupAutoDiagI(startcanid, data, datalen);
    canThread->start(QThread::HighestPriority);
}

void MainWindow::slotBaudrateIndex(int index)
{
    QString tim0 = "";
    QString tim1 = "";

    canThread->getTimerStr(index, tim0, tim1);
    ui->timer0LineEdit->setText(tim0);
    ui->timer1LineEdit->setText(tim1);
}

void MainWindow::slotTimeComboBox(int index)
{
    canThread->setShowTimeParam(index);
}

void MainWindow::slotCaseHexComboBox(int index)
{
    canThread->setShowHexParam(index);
}

void MainWindow::slotErrorInfo(int error, const QString &extend)
{
    QString text;
    switch (error) {
        case MyCanThread::ERROR_SEND:
            text.append(tr("发送失败 !"));
            break;
        case MyCanThread::ERROR_RECV:
            text.append(tr("接收失败 !"));
            break;
        case MyCanThread::ERROR_OPEN_DEVICE:
            text.append(tr("打开设备失败 !"));
            break;
        case MyCanThread::ERROR_CANINIT_DEVICE:
            text.append(tr("CAN初始化失败 !"));
            break;
        case MyCanThread::ERROR_STARTUP_DEVICE:
            text.append(tr("启动设备失败 !"));
            break;
        case MyCanThread::ERROR_CLOSE_DEVICE:
            text.append(tr("关闭设备失败 !"));
            break;
        default:
            text.append(tr("未知错误 !"));
            break;
    }

    text.append("\r\n" + extend);
    QMessageBox::critical(this, tr("thread"), text, QMessageBox::Ok);
}

void MainWindow::slotFilterDialog()
{
    if (QDialog::Accepted == filterDlg->exec()) {
        int hexLenght = 0;
        int currentMode = 0;
        uint32_t fromLowHex[100] = {0};
        uint32_t toHighHex[100] = {0};

        currentMode = filterDlg->getFilterMode();
        hexLenght = filterDlg->getFilterParam(fromLowHex, toHighHex);
        if (hexLenght > 0) {
            canThread->setFilterParam(currentMode, fromLowHex, toHighHex, hexLenght);
        }
    }
}

void MainWindow::slotProtocolSelect(int index)
{
    if (0 == index || 3 == index) {
        ui->DIAutoButton->setEnabled(false);
        ui->DIPointTriggerButton->setEnabled(false);
    } else {
        ui->DIAutoButton->setEnabled(true);
        ui->DIPointTriggerButton->setEnabled(true);
    }

    if (index > 0) {
        uint32_t reqcanid;
        uint32_t rspcanid;
    
        if (!autoDiagDlg->getCommCanid(index, &reqcanid, &rspcanid)) {
            QMessageBox::warning(this, tr("warning"), tr("没有配置交互CANID !"));
            return ;
        }
        ui->autoRadioButton->setChecked(true);
        canThread->setDeviceCanid(reqcanid, rspcanid);
        canThread->setAutoResponse(true);
        canThread->setProtocol(index);
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_F2: {
            QString opendir = tr("%1/%2").arg(QDir::currentPath(), RECORD_DIR);
            QDesktopServices::openUrl(QUrl::fromLocalFile(opendir));
            break;
        }
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    if (QMessageBox::question(this, tr("退出确认"), tr("您确定要退出该应用 ？\t"),
         QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
        event->accept(); // 不会将事件传递给组件的父组件
    } else {
        event->ignore();
    }
}

