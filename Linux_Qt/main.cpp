#include "widget.h"
#include <QApplication>
#include <QVector>
#include <QMetaType>
#include "logger.h"


int main(int argc, char *argv[])
{
    qRegisterMetaType<QVector<float>>("QVector<float>");
    Logger::instance().setLogFile("test.txt");

    QApplication a(argc, argv);
    Widget w;
    w.show();

    return a.exec();
}
