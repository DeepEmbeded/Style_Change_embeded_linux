<script setup>
import { onMounted, ref } from "vue"
import { UButton, UTextArea } from "@/components/ui"
import { IFace, ISetting } from "@/components/icons";
import { emitter, barrageShootKey } from "@/lib/mitt.js";
import Mock from "mockjs"
import { initMqtt, subscribe, publish } from "@/lib/mqtt.js";
import MarkdownIt from "markdown-it";
import LLMMessage from "@/components/live/LLMMessage.vue";

const roomId = "sWea1Y3x"
onMounted(() => {
  connect()
})

let connected = false
const MockData = Mock.mock({
  'id': '@id',
  'name': '@cname(2, 4)'
});

const userId = MockData.id
const userName = MockData.name
const message = ref("")
const messageList = ref([])
const showHint = ref(false)
const hintMsg = ref("提示信息")
const md = new MarkdownIt();
const llmId = "LLM_BACKEND"
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

const removeThinkTag = (text) => {
    const tag = "<think>";
    if (text.toLowerCase().startsWith(tag.toLowerCase())) {
      return text.substring(tag.length);
    }
    return text;
}

const connect = () => {
  const client = initMqtt()
  client.on("connect", () => {
    connected = true
    subscribe(`test/chat/${roomId}`, (message) => {
      const msgBody = JSON.parse(message)
      messageList.value.push(msgBody)
      emitter.emit(barrageShootKey, msgBody.content)
    })

    subscribe(`LLM/result`, (message) => {
      const msgBody = {
        userId: llmId,
        content: md.render(removeThinkTag(message)),
      }
      messageList.value.push(msgBody)
    })
  })
}


const sendMessage = () => {
  if (!connected) {
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
  publish(`test/chat/${roomId}`, formatedMessage)
  message.value = ""
}
</script>

<template>
  <div class="flex flex-col border rounded-sm ">
    <div class="bg-gray-100 px-2 py-5">
      消息列表
<!--      当前在线人数：{{ roomMemberCount.toString() }}-->
    </div>
    <div id="message-container" class="grow overflow-y-scroll px-2 py-3 shrink-0 h-0">
      <div
        v-for="(msg, index) in messageList"
        :key="index"
        class="mb-2 text-sm text-justify">
        <div v-if="msg.userId === llmId" id="llm-msg">
          <span class="block w-full text-center text-xl mb-2">小瑞</span>
          <LLMMessage :html-content="msg.content"/>
        </div>
        <span v-else>
          <span  class="text-gray-400">{{ msg.username }}:</span>
          <i class="inline-block w-1"></i>{{ msg.content }}
        </span>
      </div>
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

<style>

</style>
