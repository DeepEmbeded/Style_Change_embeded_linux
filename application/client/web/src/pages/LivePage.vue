<script setup>
import ChatRoom from "@/components/live/ChatRoom.vue";
import VideoElement from "@/components/live/VideoElement.vue";
</script>

<template>
  <div class="flex px-5 py-3 gap-3 min-w-[960px] max-w-[1440px] min-h-[720px]">
    <VideoElement class="w-8/10 h-full"/>
    <ChatRoom class="w-2/10 h-full"
              room-id="WxAreR" web-socket-url="http://localhost:8080/chat" />
  </div>
</template>

<style scoped></style>
