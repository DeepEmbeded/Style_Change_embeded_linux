<script setup>
import { RouterView } from "vue-router"
import MainNav from "@/components/MainNav.vue";

</script>

<template>
  <div class="flex flex-col h-screen">
    <MainNav class="h-1/16"/>
    <router-view class="grow border h-15/16 "></router-view>
  </div>
</template>

<style scoped></style>
