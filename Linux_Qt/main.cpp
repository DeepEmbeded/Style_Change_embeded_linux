#include "widget.h"
#include <QApplication>
#include <QVector>
#include <QMetaType>

int main(int argc, char *argv[])
{
    qRegisterMetaType<QVector<float>>("QVector<float>");

    QApplication a(argc, argv);
    Widget w;
    w.show();

    return a.exec();
}
