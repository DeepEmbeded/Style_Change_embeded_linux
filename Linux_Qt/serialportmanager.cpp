#include "serialportmanager.h"
#include <QDebug>

SerialPortManager::SerialPortManager(QObject *parent)
    : QObject(parent), m_serialPort(new QSerialPort(this))
{
    connect(&m_timer, &QTimer::timeout, this, &SerialPortManager::sendNext);
    m_timer.setInterval(5); // 每5ms发送一次（可调）
}

SerialPortManager::~SerialPortManager()
{
    closePort();
}

bool SerialPortManager::openPort(const QString &portName, qint32 baudRate)
{
    if (m_serialPort->isOpen())
        m_serialPort->close();

    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort->open(QIODevice::WriteOnly)) {
        emit portOpened();
        m_timer.start();
        return true;
    } else {
        emit errorOccurred(m_serialPort->errorString());
        return false;
    }
}

void SerialPortManager::closePort()
{
    m_timer.stop();
    if (m_serialPort->isOpen())
        m_serialPort->close();

    emit portClosed();
}

void SerialPortManager::sendData(const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    m_queue.enqueue(data);
}

void SerialPortManager::sendNext()
{
    QMutexLocker locker(&m_mutex);
    if (!m_queue.isEmpty() && m_serialPort->isOpen()) {
        QByteArray data = m_queue.dequeue();
        m_serialPort->write(data);
    }
}

