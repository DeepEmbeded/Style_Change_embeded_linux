import { createMemoryHistory, createRouter } from "vue-router"
import LinkPage from "./pages/LinkPage.vue"
import LivePage from "./pages/LivePage.vue"

const routes = [
  { path: "/", component: LivePage },
  { path: "/link", component: LinkPage },
  { path: "/live", component: LivePage },
]

export const router = createRouter({
  history: createMemoryHistory(),
  routes,
})
