#include <QApplication>
#include <QFont>
#include <QTextCodec>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    QTextCodec *code = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(code);
    QFont font(QString::fromLocal8Bit("微软雅黑"),10);
    a.setFont(font);
    MainWindow w;
    w.show();
    return a.exec();
}
