<script setup>
import SockJS from "sockjs-client"
import Stomp from "stompjs"
import { onMounted, ref, onUnmounted } from "vue"
import {
  SOCKET_TOPIC_RECEIVE,
  SOCKET_TOPIC_SEND
} from "@/config.js"
import { UButton, UTextArea } from "@/components/ui"
import { IFace, ISetting } from "@/components/icons";
import { emitter, barrageShootKey } from "@/lib/mitt.js";
import Mock from "mockjs"

const props = defineProps({
  webSocketUrl: {
    type: String,
    required: true,
  },
  roomId: {
    type: String,
    required: true,
  },
})
onMounted(() => {
  connect()
})
onUnmounted(() => {
  disconnect()
})

let stompClient = null
const MockData = Mock.mock({
  'id': '@id',
  'name': '@cname(2, 4)'
});
const { webSocketUrl, roomId } = props
const userId = MockData.id
const userName = MockData.name
const message = ref("")
const messageList = ref([])
const showHint = ref(false)
const hintMsg = ref("提示信息")
/**
 * 展示对用户操作的提示
 * @param msg 提示信息
 */
const showHintBlowMsgContainer = (msg) => {
  showHint.value = true
  hintMsg.value = msg
  setTimeout(()=>{
    showHint.value = false
    hintMsg.value = ""
  }, 2000)
}
/**
 * 获取格式化的时间
 * @returns {`${string}:${string}:${string}`} HH:mm:ss 格式的时间
 */
const getFormattedCurrentTime = () => {
  const currentDate = new Date();
  const hours = String(currentDate.getHours()).padStart(2, '0');
  const minutes = String(currentDate.getMinutes()).padStart(2, '0');
  const seconds = String(currentDate.getSeconds()).padStart(2, '0');
  return `${hours}:${minutes}:${seconds}`;
}
/**
 * websocket 连接 & 订阅
 */
const connect = () => {
  disconnect()
  const socket = new SockJS(webSocketUrl)
  stompClient = Stomp.over(socket)
  stompClient.connect({}, () => {
    stompClient.subscribe(SOCKET_TOPIC_RECEIVE(roomId), (roomMessage) => {
      const msgBody = JSON.parse(roomMessage.body)
      messageList.value.push(msgBody)
      emitter.emit(barrageShootKey, msgBody.content)
    })
  })
}
/**
 * websocket 断连
 */
const disconnect = () => {
  if (stompClient == null) {
    return
  }
  stompClient.disconnect()
  stompClient = null
}
/**
 * 发送消息
 */
const sendMessage = () => {
  if (stompClient == null) {
    showHintBlowMsgContainer("聊天室掉线，重新连接中");
    connect()
    return
  }
  if (message.value === "") {
    showHintBlowMsgContainer("信息不能为空")
    return
  }
  const formatedMessage = JSON.stringify({
    "userId": userId,
    "username": userName,
    "content": message.value,
    "time": getFormattedCurrentTime(),
  })
  stompClient.send(SOCKET_TOPIC_SEND(roomId), {}, formatedMessage)
  message.value = ""
}
</script>

<template>
  <div class="flex flex-col border rounded-sm overflow-hidden">
    <div class="bg-gray-100 px-2 py-5">
      消息列表
<!--      当前在线人数：{{ roomMemberCount.toString() }}-->
    </div>
    <div id="message-container" class="grow overflow-y-scroll px-2 py-3">
      <p
        v-for="(msg, index) in messageList"
        :key="index"
        class="mb-2 text-sm text-justify">
        <span class="text-gray-400">{{ msg.username }}:</span>
        <i class="inline-block w-1"></i>{{ msg.content }}
      </p>
    </div>
    <div class="flex items-center justify-center mb-3 text-sm text-red-400"
         v-show="showHint"> {{ hintMsg }} </div>
    <div class="bg-gray-200 flex flex-col px-2 py-2 gap-2">
      <u-text-area v-model="message" class="max-h-30"/>
      <div class="flex flex-row items-center gap-2">
        <IFace class="h-7 w-7 text-black hover:text-black/60 transition duration-300"/>
        <ISetting class="h-7 w-7 text-black hover:text-black/60 transition duration-300"/>
        <div class="grow"></div>
        <u-button @click="sendMessage" size="sm">发送</u-button>
      </div>
    </div>
  </div>
</template>
