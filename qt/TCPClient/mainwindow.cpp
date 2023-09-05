#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QHostAddress>
#include <QHostInfo>
#include <QMessageBox>

void MainWindow::closeEvent(QCloseEvent* event) {
    on_actDisconnect_triggered();// 退出前先关闭TCP连接
    event->accept();
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow) {
    ui->setupUi(this);

    tcpClient = new QTcpSocket(this);

    labSocketState = new QLabel("Socket状态：");
    labSocketState->setMinimumWidth(250);
    ui->statusBar->addWidget(labSocketState);

    connect(tcpClient, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(tcpClient, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(tcpClient, &QTcpSocket::stateChanged, this, &MainWindow::onSocketStateChange);
    connect(tcpClient, &QTcpSocket::readyRead, this, &MainWindow::onSocketReadyRead);
}

MainWindow::~MainWindow() {
    delete ui;
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

void MainWindow::onConnected() {  // connected()信号槽函数
    ui->logText->appendPlainText("[log] 已连接到服务器");
    ui->logText->appendPlainText("[log] peer address:" +
                                       tcpClient->peerAddress().toString());
    ui->logText->appendPlainText("[log] peer port:" +
                                       QString::number(tcpClient->peerPort()));
    ui->actConnect->setEnabled(false);
    ui->actDisconnect->setEnabled(true);
}

void MainWindow::onDisconnected() {  // disConnected()信号槽函数
    ui->logText->appendPlainText("[log] 已断开与服务器的连接");
    ui->actConnect->setEnabled(true);
    ui->actDisconnect->setEnabled(false);
}

void MainWindow::onSocketReadyRead() {  // readyRead()信号槽函数
    while (tcpClient->canReadLine())
        ui->logText->appendPlainText("[recv] " + tcpClient->readLine());
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

void MainWindow::on_actConnect_triggered() {  // 连接到服务器
    tcpClient->connectToHost(ui->lineEdit->text(), ui->spinPort->value());
}

void MainWindow::on_actDisconnect_triggered() {  // 断开与服务器的连接
    if (tcpClient->state() == QAbstractSocket::ConnectedState)
        tcpClient->disconnectFromHost();
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
    QByteArray str = msg.toUtf8();
    str.append('\n');
    tcpClient->write(str);
}
