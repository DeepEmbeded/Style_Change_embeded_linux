import { createApp } from "vue"
import "./assets/style/main.css"
import "./assets/style/index.css"
import App from "./App.vue"
import { router } from "./router.js"

createApp(App)
  .use(router)
  .mount("#app")
