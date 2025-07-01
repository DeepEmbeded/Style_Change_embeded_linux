import { createRouter, createWebHistory } from "vue-router";
import LivePage from "./pages/LivePage.vue"
import MonitorPage from "./pages/MonitorPage.vue"
import LinkPage from "./pages/LinkPage.vue"

const routes = [
  { path: "/", component: LinkPage},
  { path: "/live", component: LivePage },
  { path: "/monitor", component: MonitorPage },
]

export const router = createRouter({
  history: createWebHistory(),
  routes,
})
