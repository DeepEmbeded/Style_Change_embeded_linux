import { createMemoryHistory, createRouter } from "vue-router"
import LinkPage from "./pages/LinkPage.vue"
import LivePage from "./pages/LivePage.vue"
import MonitorPage from "./pages/MonitorPage.vue"

const routes = [
  { path: "/", component: MonitorPage },
  { path: "/link", component: LinkPage },
  { path: "/live", component: LivePage },
  { path: "/monitor", component: MonitorPage },
]

export const router = createRouter({
  history: createMemoryHistory(),
  routes,
})
