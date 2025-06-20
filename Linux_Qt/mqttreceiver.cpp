#include "mqttreceiver.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

MqttReceiver::MqttReceiver(QObject *parent) : QObject(parent) {
    client = new QMqttClient(this);

    connect(client, &QMqttClient::connected, this, [this]() {
        qDebug() << "[MQTT] Connected!";

        // ✅ 先发送一条空的保留消息来清除旧坐标
        QMqttTopicName topic("touch/coords");
        QMqttPublishProperties properties;
        client->publish(topic, QByteArray(), 0, true);  // retain = true, 清空消息


        auto sub = client->subscribe(QMqttTopicFilter("touch/coords"), 0);
        if (sub) {
            qDebug() << "[MQTT] 订阅成功: touch/coords";
        } else {
            qDebug() << "[MQTT] 订阅失败!";
        }
    });

    connect(client, &QMqttClient::messageReceived,
            this, [=](const QByteArray &message, const QMqttTopicName &topic) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(message, &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            int x = obj["x"].toInt();
            int y = obj["y"].toInt();
            qDebug() << "Received coordinates from topic" << topic.name() << ": x =" << x << ", y =" << y;
            emit coordinateReceived(x, y);
        } else {
            qDebug() << "Failed to parse JSON message:" << message;
        }
    });


    connect(client, &QMqttClient::disconnected, this, []() {
        qDebug() << "[MQTT] 与Broker断开连接";
    });
}

void MqttReceiver::connectToBroker(const QString &host, quint16 port) {
    client->setHostname(host);
    client->setPort(port);
    qDebug() << "[MQTT] 正在连接至 Broker:" << host << ":" << port;
    client->connectToHost();
}

