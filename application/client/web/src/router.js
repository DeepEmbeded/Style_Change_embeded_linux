import { createRouter, createWebHistory } from "vue-router";
import LivePage from "./pages/LivePage.vue"
import MonitorPage from "./pages/MonitorPage.vue"

const routes = [
  { path: "/live", component: LivePage },
  { path: "/monitor", component: MonitorPage },
]

export const router = createRouter({
  history: createWebHistory(),
  routes,
})
