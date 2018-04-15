# OpenLuat 

> OpenLuat 基于合宙（AirM2M) 原有Luat平台重构的一个分支，主要是重构了平台代码，增加了基于协程的多线程任务支持，使用户可以快速进行产品的开发，而不用考虑消息的繁琐回调。避免复杂的回调导致类似“goto”那种混淆的逻辑，同时保留了原有的消息回调机制。

> 当应用场景需要消息回调的时候，依旧可以使用消息的发布和订阅执行模式进行编程。


## OpenLuat -- Task 编程

- OpenLuat 支持多任务编程，利用底层消息机制和Lua原生协程完美融合实现了多线程支持和多任务编程，并且保留了消息机制特有的高实时性和低功耗特性。

- OpenLuat 提供了基于线程阻塞的函数--  `wait(ms)` ，用来帮助用户解决任务需要延时等待的情况，不同于底层rtos.sleep，调用`wait(ms)`的任务主动释放资源并挂起，直到延时值满足被主调度器恢复运行。

- OpenLuat 提供了消息机制的条件等待超时处理函数-- `result, data = waitUntil(id, ms)`,用来帮助用户解决一些需要等待条件满足立刻恢复任务的情况，并提供了超时调用回调函数的处理方式。返回值用作语句执行结束后做进一步处理用，以满足不同的场景需求。

## 在线文档

- 如果你想了解目前已经实现了那些功能，请点击：<<[参考手册](https://htmlpreview.github.io/?https://github.com/airm2m-open/Luat/master/doc/index.html)>>

## 下载地址

- 如果你想下载这个项目,请点击：[下载地址](https://github.com/airm2m-open/Luat/archive/master.zip)

# 模块介绍

## log.lua

- trace
 > 是极其细节的，例如 uart收到原始数据流，追加buffer的时候 ,这个数据可能是模块调试的时候才需要

- debug
　> 是一般细节的，例如 uart，buffer追加后的信息，进入相关parse的时候，打印buffer内容或者at指令集的交互

- info
 >　是业务级别的， 例如，打开串口了，LCD数据切换了，按键响应了等等

- warn
 > 这个级别标识有一些不应该出现的信息，不会导致程序崩溃，但是有可能运行结果不正确。
- error
 > 这个级别的错误通常会写入err文件留作分析
- fatal
 > 这个级别准备将来做hook，和errdump绑定，意图是模块只能重启， 在"死之前"将相关要搜集的信心dump到err文件中留作分析

## utils.lua
- 这个模块是一些扩展方法

## sys.lua
- 这个模块主模块，提供了任务和消息机制的管理

## console.lua
- 这个模块是串口控制台，可以利用串口在线调试模块

## clib.lua
- 这个模块是core库的扩展，对core库新增的方法在这个里面

## led.lua
- 这个模块包含几种LED的闪烁方法

## http.lua
- 这个模块包含了GET,POST,GZIP，用户鉴权等HTTP数据请求的方法

## mqtt.lua
- 这个模块包括MQTT的订阅，发布，QOS0/1/2,基于协程

## socket.lua
- 这个模块包含了tcp/udp的创建，连接，发送，关闭，基于协程