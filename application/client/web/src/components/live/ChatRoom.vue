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
import { emitter, barrageShootKey } from "@/utils/mitt.js";
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
const message = ref("测试信息")
const messageList = ref([])
const roomMemberCount = ref(1)

const getFormattedCurrentTime = () => {
  const currentDate = new Date();
  const hours = String(currentDate.getHours()).padStart(2, '0');
  const minutes = String(currentDate.getMinutes()).padStart(2, '0');
  const seconds = String(currentDate.getSeconds()).padStart(2, '0');
  return `${hours}:${minutes}:${seconds}`;
}

const connect = () => {
  disconnect()
  const socket = new SockJS(webSocketUrl)
  stompClient = Stomp.over(socket)
  stompClient.connect({}, () => {
    stompClient.subscribe(SOCKET_TOPIC_RECEIVE(roomId), (roomMessage) => {
      // console.log(roomMessage.body)
      const msgBody = JSON.parse(roomMessage.body)
      messageList.value.push(msgBody)
      emitter.emit(barrageShootKey, msgBody.content)
      // console.log(messageList.value);
    })
    // stompClient.subscribe(SOCKET_USER_INFO_RECEIVE(userId), (userMessage) => {
    //   console.log(userMessage)
    // })
    // stompClient.send(SOCKET_USER_INFO_SEND(userId), {}, "getInfo")
  })
}

const disconnect = () => {
  if (stompClient == null) {
    return
  }
  stompClient.disconnect()
  stompClient = null
}

const sendMessage = () => {
  if (stompClient == null) {
    console.log("聊天室掉线，重新连接");
    connect()
    return
  }
  if (message.value === "") {
    console.log("信息不能为空")
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
      当前在线人数：{{ roomMemberCount.toString() }}
    </div>
    <div class="grow overflow-y-scroll px-2 py-3">
      <p
        v-for="(msg, index) in messageList"
        :key="index"
        class="mb-2 text-sm text-justify">
        <span class="text-gray-400">{{ msg.username }}:</span>
        <i class="inline-block w-1"></i>{{ msg.content }}
      </p>
    </div>
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
