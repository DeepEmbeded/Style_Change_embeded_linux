import { defineConfig } from "vite";
import vue from "@vitejs/plugin-vue";
import path from "path";
import eslintPlugin from "vite-plugin-eslint";
import tailwindcss from "@tailwindcss/vite";

// https://vite.dev/config/
export default defineConfig({
  plugins: [
    vue(),
    eslintPlugin({
      include: ["src/**/*.js", "src/**/*.vue", "src/*.js", "src/*.vue"]
    }),
    tailwindcss()
  ],
  resolve: {
    alias: {
      "@": path.resolve(__dirname, "src")
    }
  },
  server: {
    host:"0.0.0.0",
  }

});


