# iot 继电器模块

- 开发环境: Platform-IO, VSCode
- 开发板： ESP-01s
- 代码基础: Arduino
- 依赖：
  - LittleFS
  - [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer.git)
  - [AsyncMqttClient](https://github.com/marvinroger/async-mqtt-client)(platformio library 中的 AsyncMqttClient-esphome@^0.8.5)
- 继电器模块：淘宝山寨货，总之很辣鸡
  - IO00: LOW 为继电器合并，HIGH 为继电器开路
  - IO02: LED 灯

## Feature

很简单的一个，继电器模块，可远程输入数字，继电器开合输入的时间。适用于类似电脑这种 按钮方式 触发的功能

- smartconfig/airkiss 配网
- 提供 http 访问
- web 下发配置(仅 MQTT 相关)
- 通过 MQTT 触发
- 通过网页按钮触发

## 注意事项

目前没提供脚本来编译、烧录固件，烧录固件还是要分成两部分
1. Build and Update 代码部分
2. Upload Filesystem Image 上传 FS 文件

通过 Platform-IO 使用 `Upload Filesystem Image` 会导致已有的配置被抹掉，修改 html 需要重新下发配置

## TODO

- 继电器模块有硬件 Bug: 通电的瞬间会闭合一下，导致断电后重新通电，模块会开合一次
- web下发配置，成功会超时，因为后边触发了重启，失败会直接提示。
