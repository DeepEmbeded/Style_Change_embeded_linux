<template>
  <input
    v-model="modelValue"
    :class="
      cn(
        'flex h-10 w-full rounded-md border border-input ' +
          'bg-background px-3 py-2 text-sm ring-offset-background file:border-0 ' +
          'file:bg-transparent file:text-foreground file:text-sm file:font-medium ' +
          'placeholder:text-muted-foreground focus-visible:outline-none focus-visible:ring-1 ' +
          'focus-visible:ring-ring disabled:cursor-not-allowed ' +
          'disabled:opacity-50',
        props.class
      )
    "
  />
</template>

<script setup>
import { cn } from "@/lib/utils.js"
import { useVModel } from "@vueuse/core"

const props = defineProps({
  defaultValue: {
    type: [String, Number],
    default: undefined,
  },
  modelValue: {
    type: [String, Number],
    default: undefined,
  },
  class: {
    type: [String, Array, Object],
    default: undefined,
  },
})

const emits = defineEmits(["update:modelValue"])

const modelValue = useVModel(props, "modelValue", emits, {
  passive: true,
  defaultValue: props.defaultValue,
})
</script>
