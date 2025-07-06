#ifndef MQTTRECEIVER_H
#define MQTTRECEIVER_H

#include <QObject>
#include <QtMqtt/QMqttClient>
#include <QFile>
#include <QTextStream>

class MqttReceiver : public QObject {
    Q_OBJECT

public:
    explicit MqttReceiver(QObject *parent = nullptr);
    void connectToBroker(const QString &host, quint16 port);

signals:
    void coordinateReceived(int x, int y);

private:
    QMqttClient *client;
    void writeLog(const QString& text);

};


#endif // MQTTRECEIVER_H
