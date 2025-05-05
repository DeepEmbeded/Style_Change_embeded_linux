<script setup>
import { ref, onMounted, onUnmounted } from "vue"
import { MediaMTXWebRTCReader } from "@/assets/webRTCReader.js"
import BulletJs from "js-bullets";
import { emitter, barrageShootKey } from "@/utils/mitt.js";
import {
  IRefresh,
  VideoPlay,
  VideoPause,
  VideoCaptionOn,
  VideoVolumeOff,
  VideoCaptionOff,
  VideoBarrageOn,
  VideoBarrageOff,
  VideoVolumeFull,
  VideoVolumeHalf,
  VideoFullScreen,
  VideoPictureToPicture
} from "@/components/icons/index.js"
import { USlider } from "@/components/ui/index.js";
import VideoFullScreenOff from "@/components/icons/FullScreenOff.vue";


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
const videoCaptionRef = ref(null)
/**
 * @type {MediaStream}
 */
let stream = null
const isPlay = ref(true)
const videoVolume = ref([100])
const videoTime = ref("")
const videoIsFullScreen = ref(false)
const videoCaptionIsOn = ref(false)
const videoBarrageIsOn = ref(true)
// const localVideoTimer = ref(0)

let screen = null
// let videoTimeInterval = null

const changeStreamTrack = (enabled) =>{
  stream.getVideoTracks().forEach((track) => {
    track.enabled = enabled
  })
  stream.getAudioTracks().forEach((track) => {
    track.enabled = enabled
  })
}

const playVideo = () => {
  if (isPlay.value) {
    videoRef.value.pause()
    changeStreamTrack(false)
  } else {
    videoRef.value.play()
    changeStreamTrack(true)
    videoRef.value.srcObject = stream;
  }
  isPlay.value = !isPlay.value
}

const switchVolume = () => {
  const muted = videoRef.value.muted
  videoRef.value.muted = !muted
  if (muted) {
    videoVolume.value = [videoRef.value.volume * 100]
  } else {
    videoVolume.value = [0]
  }
  // if (videoVolume.value[0] > 0){
  //   videoVolumeBuffer.value = videoVolume.value[0]
  //   videoVolume.value[0] = 0
  // } else {
  //   videoVolume.value[0] = videoVolumeBuffer.value
  // }
  // toggleVolume()
}

const toggleVolume = () => {
  videoRef.value.volume = videoVolume.value[0] / 100
}

const toggleCaption = () => {
  videoCaptionIsOn.value = !videoCaptionIsOn.value;
}

const toggleBarrage = () => {
  videoBarrageIsOn.value = !videoBarrageIsOn.value
}

const toggleFulScreen = () => {
  const element = videoContainerRef.value
  if (document.fullscreenElement) {
    if (document.exitFullscreen) {
      document.exitFullscreen();
    } else if (document.webkitExitFullscreen) {
      document.webkitExitFullscreen();
    } else if (document.msExitFullscreen) {
      document.msExitFullscreen();
    }
    videoIsFullScreen.value = false
  } else {
    if (element.requestFullscreen) {
      element.requestFullscreen();
    } else if (element.webkitRequestFullscreen) {
      element.webkitRequestFullscreen();
    } else if (element.msRequestFullscreen) {
      element.msRequestFullscreen();
    }
    videoIsFullScreen.value = true
  }
}

const loadVideo = () => {
  const video = videoRef.value
  video.muted = true
  video.autoplay = true
  video.playsInline = true
  videoVolume.value = [0]
  new MediaMTXWebRTCReader({
    url: new URL("whep", streamLink) + window.location.search,
    onError: () => {
      videoTime.value = "--:--"
      // clearInterval(videoTimeInterval)
      // videoTimeInterval = null
    },
    onTrack: (evt) => {
      // videoTimeInterval = setInterval(() => {refreshTime()}, 1000)
      video.srcObject = evt.streams[0]
      stream = evt.streams[0]
    },
  })
}

// const refreshTime = () => {
//   localVideoTimer.value++
//   videoTime.value = parseInt(localVideoTimer.value / 60) + ":"
//     + parseInt(localVideoTimer.value % 60)
//   // const video = videoRef.value
//   // videoTime.value = parseInt(video.currentTime / 60) + ":"
//   //   + parseInt(video.currentTime % 60)
// }

const shootBarrage = (msg) => {
  screen.push(`${msg}`)
}

const { streamLink } = props


onMounted(() => {

  screen = new BulletJs("#barrage-container", {})
  emitter.on(barrageShootKey, shootBarrage)
  loadVideo()
})

onUnmounted(() => {
  emitter.off(barrageShootKey, shootBarrage)
})
</script>

<template>
  <div class="flex flex-col border rounded-sm overflow-hidden">
<!--    直播间信息条-->
    <div class="h-[68px] shrink-0 flex items-center px-4">
      <div class="flex items-center gap-1.5 text-xl">
        <div class="rounded-full h-[48px] w-[48px] bg-gray-200"></div>
        <span>嵌入式在线教学</span>
      </div>
    </div>
    <div class="grow relative" ref="videoContainerRef">
      <video
        class="absolute top-0 left-0 w-full h-full object-contain bg-black"
        style="z-index: -999"
        ref="videoRef"
        autoplay
      >
      </video>
      <div id="video-controls-container"
           class="z-10 text-white leading-none w-full h-full absolute
           group/container flex flex-col justify-end items-center">
        <div class="bg-black/80 inline-block text-white px-4 py-2 mb-[20px]"
             ref="videoCaptionRef" v-show="videoCaptionIsOn">字幕测试</div>
        <div id="vide-controls"
             class="h-[3.8rem] bg-black/40 w-full px-[8px] bg
              justify-between hidden group-hover/container:flex">
          <div id="video-controls-left" class="flex items-center gap-1">
            <div id="control-play" @click="playVideo">
              <i v-if="isPlay"><VideoPause/></i>
              <i v-else><VideoPlay/></i>
            </div>
            <div id="volume-control" class="group/volume relative h-full flex
            items-center">
              <div @click="switchVolume">
                <i v-if="videoVolume[0] === 0"><VideoVolumeOff/></i>
                <i v-else-if="videoVolume[0] <= 50"><VideoVolumeHalf/></i>
                <i v-else><VideoVolumeFull/></i>
              </div>
              <div id="volume-control-bar"
                   class="h-[10rem] w-[48px] flex-col items-center pt-[5px]
                   pb-[12px] absolute top-[-10rem] left-[-12px] bg-black gap-1
                   hidden group-hover/volume:flex">
                <span>{{ videoVolume[0] }}</span>
                <USlider orientation="vertical" :max="100" :step="1"
                         v-model="videoVolume" @update:modelValue="toggleVolume"/>
              </div>
            </div>
            <span class="block w-[6px]"></span>
<!--            <div id="video-time" class="text-sm h-full flex items-center">-->
<!--              <span>{{ videoTime }}</span>-->
<!--            </div>-->
          </div>
          <div id="video-controls-right" class="flex items-center gap-2">
            <div id="video-barrage-control" @click="toggleBarrage">
              <i v-if="videoBarrageIsOn"><VideoBarrageOff/></i>
              <i v-else><VideoBarrageOn/></i>
            </div>
            <div id="video-caption-control" @click="toggleCaption">
              <i v-if="videoCaptionIsOn"><VideoCaptionOn/></i>
              <i v-else><VideoCaptionOff/></i>
            </div>
            <i v-show="false"><VideoPictureToPicture/></i>
            <div @click="toggleFulScreen">
              <i v-if="videoIsFullScreen"><VideoFullScreenOff/></i>
              <i v-else><VideoFullScreen/></i>
            </div>
            <i @click="loadVideo"><IRefresh/></i>
          </div>
        </div>
      </div>
      <div class="z-10 h-full w-full text-white pointer-events-none absolute
      top-0 left-0"
           id="barrage-container" v-show="videoBarrageIsOn"></div>

    </div>
    <div class="h-[90px] bg-gray-100 shrink-0"></div>
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


