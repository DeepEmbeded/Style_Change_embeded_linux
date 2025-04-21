import { createMemoryHistory, createRouter } from "vue-router"
import LinkPage from "./Pages/LinkPage.vue"
import LivePage from "./Pages/LivePage.vue"

const routes = [
  { path: "/", component: LivePage },
  { path: "/link", component: LinkPage },
  { path: "/live", component: LivePage },
]

export const router = createRouter({
  history: createMemoryHistory(),
  routes,
})
