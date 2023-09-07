#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QTcpSocket>

// 24 bytes
struct TcpParam {
    unsigned char buzzer_pin = 0;        // 1 byte
    unsigned char keyctrl_switcher = 1;  // 1 byte
    unsigned char servo_pin = 1;         // 1 byte
    struct {
        unsigned char key_pressed = 'E';  // 1 byte
        int dist = 0;                   // 4 bytes
        enum Orientation { AHEAD,
                           TURN_LEFT,
                           TURN_RIGHT,
        } orient = AHEAD;   // 4 bytes
    } motor_param;  // 12 bytes
    struct {
        float temperature = 0;
        float humidity = 0;
    } dht11_param;  // 8 bytes
};

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
    TcpParam tcpParamSend;
    TcpParam tcpParamRecv;

};

#endif  // MAINWINDOW_H
