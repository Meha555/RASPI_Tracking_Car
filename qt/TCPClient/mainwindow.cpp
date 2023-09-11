#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QHostAddress>
#include <QHostInfo>
#include <QMessageBox>
#include <QTime>

void MainWindow::closeEvent(QCloseEvent* event) {
    on_actDisconnect_triggered();// 退出前先关闭TCP连接
    event->accept();
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow) {
    ui->setupUi(this);

    tcpClient = new QTcpSocket(this);

    ui->lcdTemperature->setDecMode();
    ui->lcdTemperature->setDigitCount(4);
    ui->lcdTemperature->setSmallDecimalPoint(false);
    ui->lcdHumidity->setDecMode();
    ui->lcdHumidity->setDigitCount(4);
    ui->lcdHumidity->setSmallDecimalPoint(false);
    ui->buzzerSwitch->setEnabled(false);
    ui->keyboardSwitch->setEnabled(false);
    ui->sonarSwitch->setEnabled(false);
    ui->buzzerSwitch->setCheckable(false);
    ui->keyboardSwitch->setCheckable(false);
    ui->sonarSwitch->setCheckable(false);
    ui->btnW->setEnabled(false);
    ui->btnA->setEnabled(false);
    ui->btnS->setEnabled(false);
    ui->btnD->setEnabled(false);
    ui->btnE->setEnabled(false);

    labSocketState = new QLabel("Socket状态：");
    labSocketState->setMinimumWidth(250);
    ui->statusBar->addWidget(labSocketState);

    connect(tcpClient, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(tcpClient, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(tcpClient, &QTcpSocket::stateChanged, this, &MainWindow::onSocketStateChange);
    connect(tcpClient, &QTcpSocket::readyRead, this, &MainWindow::onSocketReadyRead);

    connect(ui->btnW, &QPushButton::clicked, this,
            [this](){tcpParamSend.motor_param.key_pressed = 'W';tcpClient->write((char*)&tcpParamSend,sizeof(TcpParam));ui->logText->appendPlainText("[send] W");},Qt::DirectConnection);
    connect(ui->btnA, &QPushButton::clicked, this,
            [this](){tcpParamSend.motor_param.key_pressed = 'A';tcpClient->write((char*)&tcpParamSend,sizeof(TcpParam));ui->logText->appendPlainText("[send] A");},Qt::DirectConnection);
    connect(ui->btnS, &QPushButton::clicked, this,
            [this](){tcpParamSend.motor_param.key_pressed = 'S';tcpClient->write((char*)&tcpParamSend,sizeof(TcpParam));ui->logText->appendPlainText("[send] S");},Qt::DirectConnection);
    connect(ui->btnD, &QPushButton::clicked, this,
            [this](){tcpParamSend.motor_param.key_pressed = 'D';tcpClient->write((char*)&tcpParamSend,sizeof(TcpParam));ui->logText->appendPlainText("[send] D");},Qt::DirectConnection);
    connect(ui->btnE, &QPushButton::clicked, this,
            [this](){tcpParamSend.motor_param.key_pressed = 'E';tcpClient->write((char*)&tcpParamSend,sizeof(TcpParam));ui->logText->appendPlainText("[send] E");},Qt::DirectConnection);

    connect(ui->buzzerSwitch, &QAbstractButton::clicked,
            this, [this](){tcpParamSend.buzzer_pin ^= 1;tcpClient->write((char*)&tcpParamSend,sizeof(TcpParam));ui->logText->appendPlainText("[send] buzzer_pin " + QString::number((int)tcpParamSend.buzzer_pin));},Qt::DirectConnection);
    connect(ui->keyboardSwitch, &QAbstractButton::clicked,
            this, [this](){tcpParamSend.keyctrl_switcher ^= 1;tcpClient->write((char*)&tcpParamSend,sizeof(TcpParam));ui->logText->appendPlainText("[send] keyctrl " + QString::number((int)tcpParamSend.keyctrl_switcher));},Qt::DirectConnection);
    connect(ui->sonarSwitch, &QAbstractButton::clicked,
            this, [this](){tcpParamSend.servo_pin ^= 1;tcpClient->write((char*)&tcpParamSend,sizeof(TcpParam));ui->logText->appendPlainText("[send] servo_pin " + QString::number((int)tcpParamSend.servo_pin));},Qt::DirectConnection);
}

MainWindow::~MainWindow() {
    delete ui;
}

void delayMsecSuspend(unsigned int msec) {
    QTime _Timer = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < _Timer );
}

QStringList MainWindow::getLocalIP() {
    QString hostName = QHostInfo::localHostName();
    QHostInfo hostInfo = QHostInfo::fromName(hostName);
    QStringList localIP;

    // 添加当前主机上的所有网卡IP
    QList<QHostAddress> addList = hostInfo.addresses();
    if (!addList.isEmpty()) {
        for (int i = 0; i < addList.count(); i++) {
            QHostAddress aHost = addList.at(i);
            if (QAbstractSocket::IPv4Protocol == aHost.protocol()) {
                localIP.append(aHost.toString());
            }
        }
    }
    return localIP;
}

void MainWindow::onSocketStateChange(QAbstractSocket::SocketState socketState) {  // stateChange()信号槽函数
    switch (socketState) {
        case QAbstractSocket::UnconnectedState:
            labSocketState->setText("Socket状态：未连接");
            break;
        case QAbstractSocket::HostLookupState:
            labSocketState->setText("Scoket状态：正在寻找主机");
            break;
        case QAbstractSocket::ConnectingState:
            labSocketState->setText("Scoket状态：正在连接");
            break;
        case QAbstractSocket::ConnectedState:
            labSocketState->setText("Scoket状态：已连接");
            break;
        case QAbstractSocket::BoundState:
            labSocketState->setText("Scoket状态：已绑定到地址和端口");
            break;
        case QAbstractSocket::ClosingState:
            labSocketState->setText("Scoket状态：正在关闭");
            break;
        case QAbstractSocket::ListeningState:
            labSocketState->setText("Scoket状态：正在监听");
    }
}

void MainWindow::onConnected() {  // connected()信号槽函数
    ui->logText->appendPlainText("[log] 已连接到服务器");
    ui->logText->appendPlainText("[log] peer address:" +
                                       tcpClient->peerAddress().toString());
    ui->logText->appendPlainText("[log] peer port:" +
                                       QString::number(tcpClient->peerPort()));
    ui->actConnect->setEnabled(false);
    ui->actDisconnect->setEnabled(true);
    ui->buzzerSwitch->setEnabled(true);
    ui->keyboardSwitch->setEnabled(true);
    ui->sonarSwitch->setEnabled(true);
    ui->buzzerSwitch->setCheckable(true);
    ui->keyboardSwitch->setCheckable(true);
    ui->sonarSwitch->setCheckable(true);
//    ui->buzzerSwitch->setChecked(false);
//    ui->keyboardSwitch->setChecked(false);
//    ui->sonarSwitch->setChecked(false);
    ui->btnW->setEnabled(true);
    ui->btnA->setEnabled(true);
    ui->btnS->setEnabled(true);
    ui->btnD->setEnabled(true);
    ui->btnE->setEnabled(true);
}

void MainWindow::onDisconnected() {  // disConnected()信号槽函数
    ui->logText->appendPlainText("[log] 已断开与服务器的连接");
    ui->actConnect->setEnabled(true);
    ui->actDisconnect->setEnabled(false);
    ui->buzzerSwitch->setEnabled(false);
    ui->keyboardSwitch->setEnabled(false);
    ui->sonarSwitch->setEnabled(false);
    ui->btnW->setEnabled(false);
    ui->btnA->setEnabled(false);
    ui->btnS->setEnabled(false);
    ui->btnD->setEnabled(false);
    ui->btnE->setEnabled(false);
}

void MainWindow::onSocketReadyRead() {  // readyRead()信号槽函数
    tcpClient->read((char*)&tcpParamRecv,sizeof(TcpParam));
    ui->logText->appendPlainText("[recv] " + QString::number(sizeof(TcpParam)) + " bytes");
    ui->buzzerSwitch->setChecked(tcpParamRecv.buzzer_pin);
    ui->keyboardSwitch->setChecked(tcpParamRecv.keyctrl_switcher);
    ui->sonarSwitch->setChecked(tcpParamRecv.servo_pin);
    ui->lcdTemperature->display(tcpParamRecv.dht11_param.temperature);
    ui->lcdHumidity->display(tcpParamRecv.dht11_param.humidity);
    ui->lcdDist->display(tcpParamRecv.motor_param.dist);
}

void MainWindow::on_actConnect_triggered() {  // 连接到服务器
    tcpClient->connectToHost(ui->lineEdit->text(), ui->spinPort->value());
}

void MainWindow::on_actDisconnect_triggered() {  // 断开与服务器的连接
    if (tcpClient->state() == QAbstractSocket::ConnectedState) {
        ui->logText->appendPlainText("发送关闭请求");
        tcpClient->write(QString("close").toUtf8().append('\0'));
//        disconnect(tcpClient, &QTcpSocket::readyRead, this, &MainWindow::onSocketReadyRead);
        tcpClient->disconnectFromHost();
//        while(tcpClient->canReadLine()){
//            if(tcpClient->readLine(4) == "ack")
//                tcpClient->disconnectFromHost();
//            else {
//                ui->logText->appendPlainText("关闭失败");
//                connect(tcpClient, &QTcpSocket::readyRead, this, &MainWindow::onSocketReadyRead);
//            }
//        }
    }
}

void MainWindow::on_actClear_triggered() {
    ui->logText->clear();
}

void MainWindow::on_btnSend_clicked() {  // 发送数据
    if(ui->lineEdit->text() == ""){
        QMessageBox::warning(this,"未填写IP","请先填写IP");
        return;
    }
    QString msg = ui->editMsg->text();
    ui->logText->appendPlainText("[send] " + msg);
    ui->editMsg->clear();
    ui->editMsg->setFocus();
    ui->buzzerSwitch->setEnabled(true);
    QByteArray str = msg.toUtf8();
    str.append('\n');
    tcpClient->write(str);
}
