<script setup>
import { RouterView } from "vue-router"
// import MainNav from "@/components/MainNav.vue";

</script>

<template>
  <div class="flex flex-col h-screen">
    <div class="grow w-full">
      <router-view class="h-full m-auto"></router-view>
    </div>
  </div>
</template>

<style scoped></style>
