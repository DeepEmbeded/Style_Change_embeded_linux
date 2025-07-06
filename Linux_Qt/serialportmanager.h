#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QQueue>
#include <QTimer>
#include <QMutex>

class SerialPortManager : public QObject
{
    Q_OBJECT
public:
    explicit SerialPortManager(QObject *parent = nullptr);
    ~SerialPortManager();

    bool openPort(const QString &portName, qint32 baudRate = QSerialPort::Baud115200);
    void closePort();

signals:
    void errorOccurred(const QString &error);
    void portOpened();
    void portClosed();


public slots:
    void sendData(const QByteArray &data);
    void sendNext(); // 定时发送

private:
    QSerialPort *m_serialPort;
    QQueue<QByteArray> m_queue;
    QTimer m_timer;
    QMutex m_mutex;
};

#endif // SERIALPORTMANAGER_H
