<template>
  <video
    width="960"
    height="540"
    ref="videoRef"
    controls
  ></video>
  <div ref="messageRef"></div>
</template>

<style scoped>
video {
  display: block;
  width: 100%;
}
</style>

<script setup>
import { ref, onMounted } from "vue"
import { MediaMTXWebRTCReader } from "@/assets/webRTCReader.js";

const props = defineProps({
  streamLink:{
    type: String,
    default: 'http://localhost:8889/mystream/'
  }
})


/**
 * @type {import('vue').Ref<HTMLVideoElement>}
 */
const videoRef = ref(null)
/**
 * @type {import('vue').Ref<HTMLElement>}
 */
const messageRef = ref(null)

const { streamLink } = props

onMounted(() => {
  const video = videoRef.value;
  const message = messageRef.value;
  let defaultControls = false;

  const setMessage = (str) => {
    if (str !== '') {
      video.controls = false;
    } else {
      video.controls = defaultControls;
    }
    message.innerText = str;
  };

  const parseBoolString = (str, defaultVal) => {
    str = (str || '');

    if (['1', 'yes', 'true'].includes(str.toLowerCase())) {
      return true;
    }
    if (['0', 'no', 'false'].includes(str.toLowerCase())) {
      return false;
    }
    return defaultVal;
  };

  const loadAttributesFromQuery = () => {
    const params = new URLSearchParams(window.location.search);
    video.controls = parseBoolString(params.get('controls'), true);
    video.muted = parseBoolString(params.get('muted'), true);
    video.autoplay = parseBoolString(params.get('autoplay'), true);
    video.playsInline = parseBoolString(params.get('playsinline'), true);
    defaultControls = video.controls;
  };


  loadAttributesFromQuery();

  new MediaMTXWebRTCReader({
    url: new URL('whep', streamLink) + window.location.search,
    onError: (err) => {
      setMessage(err);
    },
    onTrack: (evt) => {
      setMessage('');
      video.srcObject = evt.streams[0];
    },
  });

})


</script>