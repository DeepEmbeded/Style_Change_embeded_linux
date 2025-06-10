# 说明

## 流媒体服务器
- 在该文件夹下运行 `docker-compose up -d` 即可启动流媒体服务器


docker run -d --name mosquitto -p 1883:1883 -p 9001:9001 -v ${PWD}/mqtt/mosquitto.conf:/mosquitto/config/mosquitto.conf eclipse-mosquitto:2.0.21