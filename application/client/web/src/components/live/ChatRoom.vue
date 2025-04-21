<script setup>
import SockJS from "sockjs-client"
import Stomp from "stompjs"
import { onMounted, ref, onUnmounted } from "vue"
import {
  SOCKET_TOPIC_RECEIVE,
  SOCKET_TOPIC_SEND,
  SOCKET_USER_INFO_RECEIVE,
  SOCKET_USER_INFO_SEND,
} from "@/config.js"
import UButton from "@/components/ui/UButton.vue"
import UTextArea from "@/components/ui/UTextArea.vue";

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
const { webSocketUrl, roomId } = props
const userId = "MTc0NTA1MDkzNzY4Ny02NzA2"
const message = ref("测试信息")
const messageList = ref([
    {
      "username": "Alice",
      "content": "今天的天气看起来很不错呢。",
      "time": "10:15:23"
    },
    {
      "username": "Bob",
      "content": "是啊，很适合出去走走。你们有什么计划吗？",
      "time": "10:16:40"
    },
    {
      "username": "Charlie",
      "content": "我打算去公园逛逛，顺便拍些照片。",
      "time": "10:17:12"
    },
    {
      "username": "David",
      "content": "听起来不错呀，记得分享照片哦。",
      "time": "10:18:05"
    },
    {
      "username": "Eve",
      "content": "我可能会在家看电影，最近发现了几部好片。",
      "time": "10:19:30"
    },
    {
      "username": "Frank",
      "content": "哇，有什么推荐的吗？我也喜欢看电影。",
      "time": "10:20:08"
    },
    {
      "username": "Grace",
      "content": "我觉得《肖申克的救赎》就很不错，百看不厌。",
      "time": "10:21:15"
    },
    {
      "username": "Hank",
      "content": "经典中的经典，确实值得一看再看。",
      "time": "10:22:02"
    },
    {
      "username": "Alice",
      "content": "除了电影，大家平时还喜欢做些什么呢？",
      "time": "10:23:35"
    },
    {
      "username": "Bob",
      "content": "我喜欢运动，比如篮球和跑步。",
      "time": "10:24:10"
    }
])
const roomMemberCount = ref(1)

const connect = () => {
  disconnect()
  const socket = new SockJS(webSocketUrl)
  stompClient = Stomp.over(socket)
  stompClient.connect({}, () => {
    stompClient.subscribe(SOCKET_TOPIC_RECEIVE(roomId), (roomMessage) => {
      console.log(roomMessage.body)
      messageList.value.push(roomMessage.body)
    })
    stompClient.subscribe(SOCKET_USER_INFO_RECEIVE(userId), (userMessage) => {
      console.log(userMessage)
    })
    stompClient.send(SOCKET_USER_INFO_SEND(userId), {}, "getInfo")
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
  }
  if (message.value === "") {
    console.log("信息不能为空")
    return
  }
  stompClient.send(SOCKET_TOPIC_SEND(roomId), {}, message.value)
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
        {{ msg.username }}: {{ msg.content }}
      </p>
    </div>
    <div class="bg-gray-200 flex flex-col px-2 py-2 gap-2">
      <u-text-area v-model="message" class="max-h-30"/>
      <u-button @click="sendMessage" size="sm">发送</u-button>
    </div>
  </div>
</template>
