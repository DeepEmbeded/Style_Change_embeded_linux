import { createRouter, createWebHistory } from "vue-router";
import LivePage from "./pages/LivePage.vue";
import MonitorPage from "./pages/MonitorPage.vue";
import LinkPage from "./pages/LinkPage.vue";

const routes = [
  {
    path: "/", component: LinkPage, meta: {
      title: "导航页"
    }
  },
  {
    path: "/live", component: LivePage, meta: {
      title: "直播间-嵌入式在线教学"
    }
  },
  {
    path: "/monitor", component: MonitorPage, meta: {
      title: "监控端"
    }
  }
];

export const router = createRouter({
  history: createWebHistory(),
  routes
});

router.beforeEach((to, from, next) => {
  if (to.meta.title) {
    document.title = to.meta.title;
  }
  next();
});
