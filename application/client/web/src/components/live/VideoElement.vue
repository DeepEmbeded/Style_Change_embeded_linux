<script setup>
import { ref, onMounted, onUnmounted } from "vue"
import { MediaMTXWebRTCReader } from "@/assets/webRTCReader.js"
import BulletJs from "js-bullets";
import { emitter, barrageShootKey } from "@/utils/mitt.js";



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
let screen = null

const shootBarrage = (msg) => {
  screen.push(`${msg}`)
}

const { streamLink } = props

onMounted(() => {
  const video = videoRef.value
  screen = new BulletJs("#barrage-container", {})
  emitter.on(barrageShootKey, shootBarrage)

  const setMessage = (str) => {
    console.log(str);
  }

  const parseBoolString = (str, defaultVal) => {
    str = str || ""

    if (["1", "yes", "true"].includes(str.toLowerCase())) {
      return true
    }
    if (["0", "no", "false"].includes(str.toLowerCase())) {
      return false
    }
    return defaultVal
  }

  const loadAttributesFromQuery = () => {
    const params = new URLSearchParams(window.location.search)
    video.controls = parseBoolString(params.get("controls"), true)
    video.muted = parseBoolString(params.get("muted"), true)
    video.autoplay = parseBoolString(params.get("autoplay"), true)
    video.playsInline = parseBoolString(params.get("playsinline"), true)

  }

  loadAttributesFromQuery()

  new MediaMTXWebRTCReader({
    url: new URL("whep", streamLink) + window.location.search,
    onError: (err) => {
      setMessage(err)
    },
    onTrack: (evt) => {
      setMessage("")
      video.srcObject = evt.streams[0]
    },
  })
})

onUnmounted(() => {
  emitter.off(barrageShootKey, shootBarrage)
})
</script>

<template>
  <div class="flex flex-col border rounded-sm overflow-hidden">
<!--    直播间信息条-->
    <div class="h-3/32 shrink-0 flex items-center px-4">
      <div class="flex items-center gap-3 text-xl">
        <div class="rounded-full h-14 w-14 bg-gray-200"></div>
        <span>主播名</span>
      </div>
    </div>
    <div class="h-25/32 relative">
      <video
        class="absolute top-0 left-0 w-full h-full object-contain bg-gray-900"
        style="z-index: -999"
        ref="videoRef"
        controls
      >
        <source src="https://www.runoob.com/try/demo_source/mov_bbb.mp4"
                type="video/mp4" >
      </video>
      <div class="z-10 h-full w-full text-white pointer-events-none"
           id="barrage-container"></div>
      <div class="z-10" style="display: none">这是提示信息</div>
    </div>
    <div class="h-4/32 bg-gray-100 shrink-0"></div>
  </div>
</template>


