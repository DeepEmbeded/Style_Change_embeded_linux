<script setup>
import { ref, onMounted } from "vue"
import { MediaMTXWebRTCReader } from "@/lib/webRTCReader.js"
import {initMqtt, publish, subscribe} from "@/lib/mqtt.js"
import { UButton } from "@/components/ui/index.js";


const props = defineProps({
  streamLink: {
    type: String,
    default: "http://localhost:8889/mystream/",
  },
})

/**
 * @type {import('vue').Ref<HTMLVideoElement>}
 */
const videoRef = ref(null)
/**
 * @type {import('vue').Ref<HTMLElement>}
 */
const videoContainerRef = ref(null)
/**
 * @type {import('vue').Ref<HTMLElement>}
 */
const videoClickRef = ref(null)
const { streamLink } = props
const connected = ref(false);


const connect = () => {
  const client = initMqtt()
  client.on("connect", () => {
    connected.value = true;
    subscribe("test/topic", (message) => {
      console.log(message)
    })
  })
}


/**
 * 加载流媒体视频
 */
const loadVideo = () => {
  const video = videoRef.value
  video.muted = true
  video.autoplay = true
  video.playsInline = true

  new MediaMTXWebRTCReader({
    url: new URL("whep", streamLink) + window.location.search,
    onError: () => {
    },
    onTrack: (evt) => {

      video.srcObject = evt.streams[0]
    },
  })
}

const videoClick = () => {
  console.log("videoClick");
}

const panelClick = (e) => {
  e.stopPropagation()
  const offsetX = e.offsetX
  const offsetY = e.offsetY
  // const elementX = videoClickRef.value.offsetWidth
  // const elementY = videoClickRef.value.offsetHeight
  if (connected.value) {
    publish("test/topic", JSON.stringify({x: offsetX, y: offsetY}))
  } else {
    connect()
  }
}


onMounted(() => {
  loadVideo()
  connect()
})

</script>

<template>
  <div class="flex align-center justify-center w-full h-full gap-0.5">
    <div class="relative min-w-[1172px] min-h-[659px]" ref="videoContainerRef">
      <video
        class="absolute top-0 left-0 bg-black min-w-[1172px] min-h-[659px]"
        style="z-index: -999"
        ref="videoRef"
        autoplay
        @click="videoClick"
      ></video>
      <div
        class="absolute top-0 left-0 min-w-[1172px] min-h-[659px]"
        ref="videoClickRef"
        @click="panelClick"
      ></div>
    </div>
    <div><UButton>刷新选择</UButton></div>
  </div>
</template>

<style scoped>
i>svg{
  --width: 24px;
  display: inline-block;
  height: var(--width);
  width: var(--width);
}
</style>


