```
nginx -t -p /etc/nginx -c nginx.conf  # 测试配置
nginx -s stop && nginx -p /etc/nginx  # 重启生效
```

```
vi /etc/nginx/nginx.conf
```

```
mkdir -p /var/log/nginx/         
mkdir -p /var/cache/nginx/client-body  
chown -R www-data:www-data /var/log/nginx/ 
chown -R www-data:www-data /var/cache/nginx/  
chmod -R 755 /var/log/nginx/
chmod -R 755 /var/cache/nginx/
```

```
gst-launch-1.0 \
  v4l2src device=/dev/video11 ! \
  video/x-raw,format=NV12,width=640,height=480,framerate=30/1 ! \
  videoconvert ! \
  mpph264enc ! \
  h264parse ! \
  flvmux ! \
  rtmpsink location="rtmp://192.168.10.50/live/stream"
```

![image-20250522170757302](C:\Users\13227\Documents\GitHub\Style_Change_embeded_linux\image-20250522170757302.png)