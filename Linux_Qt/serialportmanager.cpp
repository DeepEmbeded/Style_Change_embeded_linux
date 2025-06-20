#include "serialportmanager.h"
#include <QDebug>

SerialPortManager::SerialPortManager(QObject *parent)
    : QObject(parent), m_serialPort(new QSerialPort(this))
{
    connect(m_serialPort, &QSerialPort::errorOccurred, this, [=](QSerialPort::SerialPortError error) {
        if (error != QSerialPort::NoError)
            emit errorOccurred(m_serialPort->errorString());
    });
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
        return true;
    } else {
        emit errorOccurred("Failed to open port: " + m_serialPort->errorString());
        return false;
    }
}

void SerialPortManager::closePort()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        emit portClosed();
    }
}

void SerialPortManager::sendData(const QByteArray &data)
{
    if (m_serialPort->isOpen()) {
        m_serialPort->write(data);
    }
}
