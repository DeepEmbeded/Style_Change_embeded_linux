<script setup>
import { RouterLink } from "vue-router";
import VideoElement from "@/components/VideoElement.vue";
</script>

<template>
LinkPage
  <VideoElement/>
  <router-link to="/live">
    LivePage
  </router-link>
</template>

<style scoped>

</style>