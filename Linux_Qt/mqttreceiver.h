#ifndef MQTTRECEIVER_H
#define MQTTRECEIVER_H

#include <QObject>
#include <QtMqtt/QMqttClient>


class MqttReceiver : public QObject {
    Q_OBJECT

public:
    explicit MqttReceiver(QObject *parent = nullptr);
    void connectToBroker(const QString &host, quint16 port);

signals:
    void coordinateReceived(int x, int y);

private:
    QMqttClient *client;
};


#endif // MQTTRECEIVER_H
