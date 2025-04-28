<script setup>
import { ref, onMounted, onUnmounted } from "vue"
import { MediaMTXWebRTCReader } from "@/assets/webRTCReader.js"
import BulletJs from "js-bullets";
import { emitter, barrageShootKey } from "@/utils/mitt.js";
import {
  IRefresh,
  VideoPlay,
  VideoPause,
  VideoVolumeOff,
  VideoVolumeFull,
  VideoVolumeHalf,
  VideoFullScreen,
  VideoPictureToPicture
} from "@/components/icons/index.js"
import { USlider } from "@/components/ui/index.js";

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


const play = ref(false)
const videoVolume = ref([100])
const videoTime = ref("")

let screen = null

const playVideo = () => {
  if (play.value) {
    videoRef.value.pause()
    play.value = false
  } else {
    videoRef.value.play()
    play.value = true
  }
}

// const toggleVolume = () => {
//   if (volumeOn.value) {
//     videoRef.value.volume = 0
//
//   }
// }

const refreshTime = () => {
  const video = videoRef.value
  videoTime.value = parseInt(video.currentTime / 60) + ":"
    + parseInt(video.currentTime % 60) + "/"
    + parseInt(video.duration / 60) + ":"
    + parseInt(video.duration % 60);
}

const shootBarrage = (msg) => {
  screen.push(`${msg}`)
}

const { streamLink } = props

onMounted(() => {
  const video = videoRef.value
  screen = new BulletJs("#barrage-container", {})
  emitter.on(barrageShootKey, shootBarrage)

  video.onloadedmetadata = refreshTime
  setInterval(() => {refreshTime()}, 1000)

  const setMessage = (str) => {
    console.log(str);
  }

  /**

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
    **/

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
        <span>嵌入式在线教学</span>
      </div>
    </div>
    <div class="h-25/32 relative">
      <video
        class="absolute top-0 left-0 w-full h-full object-contain bg-gray-900"
        style="z-index: -999"
        ref="videoRef"
      >
        <source src="https://www.runoob.com/try/demo_source/mov_bbb.mp4"
                type="video/mp4" >
      </video>
      <div id="video-controls-container"
           class="z-10 text-white leading-none w-full h-full relative
           group/container">
        <div id="vide-controls"
             class="h-[3.8rem] bg-black/40 w-full px-[8px] bg bottom-0
             absolute justify-between hidden group-hover/container:flex">
          <div id="video-controls-left" class="flex items-center gap-1">
            <div id="control-play" @click="playVideo">
              <i v-if="play"><VideoPause/></i>
              <i v-else><VideoPlay/></i>
            </div>
            <div id="volume-control relative" class="group/volume h-full flex items-center">
              <i v-if="videoVolume[0] === 0"><VideoVolumeOff/></i>
              <i v-else-if="videoVolume[0] <= 50"><VideoVolumeHalf/></i>
              <i v-else><VideoVolumeFull/></i>
              <div id="volume-control-bar"
                   class="h-[10rem] w-[48px] flex-col items-center pt-[5px]
                   pb-[12px] absolute top-[-10rem] left-[24px] bg-black gap-1
                   hidden group-hover/volume:flex">
                <span>{{ videoVolume[0] }}</span>
                <USlider orientation="vertical" :max="100" :step="1"
                         v-model="videoVolume"/>
              </div>
            </div>
            <span class="block w-[6px]"></span>
            <div id="video-time" class="text-sm h-full flex items-center">
              <span>{{ videoTime }}</span>
            </div>
          </div>
          <div id="video-controls-right" class="flex items-center gap-2">
            <i><VideoPictureToPicture/></i>
            <i><VideoFullScreen/></i>
            <i><IRefresh/></i>
          </div>
        </div>
      </div>
      <div class="z-10 h-full w-full text-white pointer-events-none"
           id="barrage-container"></div>
      <div class="z-10" style="display: none">这是提示信息</div>
    </div>
    <div class="h-4/32 bg-gray-100 shrink-0"></div>
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


