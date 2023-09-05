#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QTcpSocket>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    QTcpSocket* tcpClient;   // socket
    QLabel* labSocketState;  // 状态栏显示标签
    QStringList getLocalIP();  // 获取本机IP地址

protected:
    void closeEvent(QCloseEvent* event) override;

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

private slots:
    void onConnected();
    void onDisconnected();
    void onSocketStateChange(QAbstractSocket::SocketState socketState);
    void onSocketReadyRead();  // 读取socket传入的数据
    void on_actConnect_triggered();
    void on_actDisconnect_triggered();
    void on_actClear_triggered();
    void on_btnSend_clicked();

private:
    Ui::MainWindow* ui;
};

#endif  // MAINWINDOW_H
