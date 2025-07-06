# 项目测试手册

## 前端项目环境
nodejs: 22.x

包管理器: 推荐使用 pnpm 作为包管理器
建议使用 webstrom 作为 IDE 获得更好的提示与支持

pnpm [安装](https://www.pnpm.cn/installation):
```bash
npm install -g pnpm@latest-10
```
安装依赖:

```bash
cd application/client/web
pnpm install
```

测试项目:
```bash
pnpm run dev
```

打包:
```bash
pnpm run build
# 运行该命令会在项目中生成一个 dist 文件夹，里面是打包好的网站代码，可用 nginx 部署
```

预览:
```bash
# 打包之后才可以预览
pnpm run preview
```

### 各种依赖说明
`@`开头的一般都是类型文件，用于 IDE 的智能提示
```json
"dependencies": {
  "@tailwindcss/vite": "^4.1.4",
  "@vueuse/core": "^13.1.0",
  "class-variance-authority": "^0.7.1", // UI 使用，参考 src/components/ui 中的代码
  "clsx": "^2.1.1",                     // UI 使用
  "js-bullets": "^1.2.0",
  "markdown-it": "^14.1.0",             // Markdown 文本解析
  "mitt": "^3.0.1",                     // 兄弟/父子组件间通信，参考 src/components/live/ChatRoom.vue 中的使用
  "mockjs": "^1.1.0",                   // 随机假数据生成
  "tailwind-merge": "^3.2.0",           // UI 使用
  "vue": "^3.5.13",                     // vue 框架
  "vue-router": "4"                     // 路由，参考 src/router.js 中的使用
},
"devDependencies": {
  "@babel/core": "^7.26.10",
  "@babel/eslint-parser": "^7.27.0",
  "@eslint/js": "^9.24.0",
  "@types/mockjs": "^1.0.10",
  "@types/sockjs-client": "^1.5.4",
  "@types/stompjs": "^2.3.9",
  "@vitejs/plugin-vue": "^5.2.1",
  "eslint": "^9.24.0",                  // 代码规范
  "eslint-config-prettier": "^10.1.2",
  "eslint-plugin-prettier": "^5.2.6",
  "eslint-plugin-vue": "^10.0.0",
  "globals": "^16.0.0",                 // 其他包的依赖
  "prettier": "^3.5.3",                 // 代码格式化
  "reka-ui": "^2.2.0",                  // UI
  "sockjs-client": "^1.6.1",            // websocket 通信使用
  "stompjs": "^2.3.3",                  // websocket 通信使用
  "tailwindcss": "^4.1.4",              // UI
  "tw-animate-css": "^1.2.6",           // UI 动画
  "vite": "^6.2.0",                     // 打包器
  "vite-plugin-eslint": "^1.8.1"
}
```

### 项目文件说明
```txt
/src/assets         目前只有 css 文件  
/src/components     vue 组件  
/src/lib            项目内使用的 js  
/src/pages          vue 页面  
/src/App.vue        入口文件  
/src/config.js      项目内使用的静态配置字段  
/src/main.js        主程序  
/src/router.js      vue 路由配置文件  
.eslintrc.js        eslint 配置文件 用于 IDE  
.jsconfig js        js 文件说明  
.prettierrc         prettier 配置文件  
eslint.config.js    eslint 配置文件 用于打包器   
```

### 其他
- 项目使用了 tailwindcss 写样式，样式基本都写在标签上，`src/assets/style/main.css` 可以控制 `src/components/ui` 中组件的颜色配置，`src/assets/style/index.css` 用于控制全局样式 

## 流媒体服务器
docker 启动:
```bash
docker run --rm -it -e MTX_RTSPTRANSPORTS=tcp -e MTX_WEBRTCADDITIONALHOSTS=192.168.137.60 -p 8554:8554 -p 1935:1935 -p 8888:8888 -p 8889:8889 -p 8890:8890/udp -p 8189:8189/udp bluenviron/mediamtx
```
`MTX_WEBRTCADDITIONALHOSTS` 设置成自己主机的 ip 地址

ffmpeg 推流
```bash
ffmpeg -stream_loop -1 -re -i trailer.mp4 -c:v libx264 -preset medium -tune zerolatency -c:a libopus -b:a 128k -f rtsp rtsp://localhost:8554/mystream
```
运行起来可能会出现各种问题，修改参数多试几遍
启动流媒体服务器之后，向流媒体服务器推流才能在前端的直播间拉流

目前发现的问题：
1. 流媒体服务器无法使用 docker-compose 配置启动，推测是网络环境的问题，或者说是 windows 版 docker desktop 的问题。

## MQTT 服务器
在 application\devops 目录下打开终端，运行命令 `docker-compose up` 即可启动 MQTT 服务器


