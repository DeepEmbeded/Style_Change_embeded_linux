// src/utils/mqtt.js
import mqtt from 'mqtt'

let client = null

// 无认证配置
const config = {
  host: 'ws://localhost:9001',  // 使用 WebSocket 协议（重要！）
  connectTimeout: 3000,
  reconnectPeriod: 1000,
  clean: true
}

export function initMqtt() {
  if (client) return client

  // 创建无认证客户端
  client = mqtt.connect(config.host, {
    connectTimeout: config.connectTimeout,
    reconnectPeriod: config.reconnectPeriod,
    clean: config.clean
  })

  // 事件监听
  client.on('connect', () => console.log('MQTT 连接成功'))
  client.on('error', (err) => console.error('MQTT 连接错误:', err))
  client.on('reconnect', () => console.log('MQTT 重连中...'))
  client.on('offline', () => console.log('MQTT 已离线'))

  return client
}

// 订阅主题
export function subscribe(topic, callback) {
  if (!client) initMqtt()
  client.subscribe(topic, { qos: 0 }, (err) => {
    if (err) console.error('订阅失败:', err)
  })
  client.on('message', (receivedTopic, payload) => {
    if (receivedTopic === topic) {
      callback(payload.toString())
    }
  })
}

// 发布消息
export function publish(topic, message) {
  if (!client) initMqtt()
  client.publish(topic, message, { qos: 0 }, (err) => {
    if (err) console.error('发布失败:', err)
  })
}