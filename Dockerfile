FROM nginx:alpine
COPY webassembly.js webassembly.js.map index.js index.html main.wasm /usr/share/nginx/html/
