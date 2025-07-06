#include "mqttreceiver.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

MqttReceiver::MqttReceiver(QObject *parent) : QObject(parent) {
    client = new QMqttClient(this);

    connect(client, &QMqttClient::connected, this, [this]() {
        QString msg = "[MQTT] Connected!";
        qDebug() << msg;
        writeLog(msg);

        // ✅ 先发送一条空的保留消息来清除旧坐标
        QMqttTopicName topic("touch/coords");
        QMqttPublishProperties properties;
        client->publish(topic, QByteArray(), 0, true);  // retain = true, 清空消息

        auto sub = client->subscribe(QMqttTopicFilter("touch/coords"), 0);
        if (sub) {
            msg = "[MQTT] sub success: touch/coords";
            qDebug() << msg;
            writeLog(msg);
        } else {
            msg = "[MQTT] sub fail!";
            qDebug() << msg;
            writeLog(msg);
        }
    });

    connect(client, &QMqttClient::messageReceived, this,
            [this](const QByteArray &message, const QMqttTopicName &topic) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(message, &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            int x = obj["x"].toInt();
            int y = obj["y"].toInt();
            QString log = QString("Received coordinates from topic %1: x = %2, y = %3")
                            .arg(topic.name(), QString::number(x), QString::number(y));
            qDebug() << log;
            writeLog(log);
            emit coordinateReceived(x, y);
        } else {
            QString errLog = QString("Failed to parse JSON message: %1")
                             .arg(QString::fromUtf8(message));
            qDebug() << errLog;
            writeLog(errLog);
        }
    });

    connect(client, &QMqttClient::disconnected, this, [this]() {
        QString msg = "[MQTT] 与Broker断开连接";
        qDebug() << msg;
        writeLog(msg);
    });
}

void MqttReceiver::connectToBroker(const QString &host, quint16 port) {
    client->setHostname(host);
    client->setPort(port);
    qDebug() << "[MQTT] 正在连接至 Broker:" << host << ":" << port;
    client->connectToHost();
}

void MqttReceiver::writeLog(const QString& text)
{
    QFile file("mqtt.log");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        out << "[" << time << "] " << text << "\n";
        file.close();
    }
}


