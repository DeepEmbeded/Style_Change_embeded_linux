#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>

class SerialPortManager : public QObject
{
    Q_OBJECT
public:
    explicit SerialPortManager(QObject *parent = nullptr);
    ~SerialPortManager();

    bool openPort(const QString &portName, qint32 baudRate = QSerialPort::Baud115200);
    void closePort();
    void sendData(const QByteArray &data);
signals:
    void errorOccurred(const QString &error);
    void portOpened();
    void portClosed();

public slots:

private:
    QSerialPort *m_serialPort;
};

#endif // SERIALPORTMANAGER_H
