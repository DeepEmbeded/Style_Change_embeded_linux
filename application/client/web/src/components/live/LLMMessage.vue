<script>
export default {
  props: {
    htmlContent: String
  }
}
</script>
<template>
  <div class="llm-msg bg-blue-100 p-2 rounded" v-html="htmlContent"/>
</template>

<style >
@reference "@/assets/style/main.css"
.llm-msg {
  @apply bg-blue-300 p-2 box-border !important;
}

.llm-msg h1,h2,h3,h4,h5,h6 {
  @apply font-bold;
}

.llm-msg ul li{
  list-style: disc inside;
}

.llm-msg pre code {
  @apply bg-gray-200 rounded box-border;
}
</style>