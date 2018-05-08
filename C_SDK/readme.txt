Luat_Iot_SDK_C语言开发环境，用于air202模块进行C语言二次开发，丰富的api接口。更多内容查阅doc目录



V1.1修改记录
-----------------------
1. 增加timer，fs，flash，gpio，alarm，uart，audio 接口对应demo
2. 增加udp收发demo
3. 修改RAM空间为1MB
4. 增加iot_debug_set_fault_mode接口，用来设置软件异常情况下，是否进入调试模式
5. 增加iot_audio_rec_start/stop录音接口
6. 解决tcp/udp连续发送导致的死机问题
7. 更新《Luat_IOT_SDK_C语言下载调试手册》《Luat_IoT_SDK_C语言编程手册》


V1.2修改记录
-----------------------
1. 修改udp连续发送死机问题
2. 修改select对端关闭后，没有返回的问题
3. 解决服务器关闭时，依然能链接成功的问题
4. 合入RDA8955_W17.27.5_IDH
5. 增加FTP应用
6. 增加OTA（远程升级APP）demo
7. 更新《Luat_IOT_SDK_C语言下载调试手册》《Luat_IoT_SDK_C语言编程手册》


V1.3修改记录
-----------------------
1. 基础版本升级
2. 解决录音不连续的问题
3. 增加demo_minisystem项目
4. 增加默认./demo_minisystem_B4245.lod文件，用来测试验证
5. 更新《Luat_IoT_SDK_C语言环境安装步骤》《Luat_IoT_SDK_C语言编程手册》

V1.3.1修改记录
-----------------------
1. 增加虚拟AT接口iot_vat_init、iot_vat_sendATcmd以及相关例子

V1.3.2修改记录
-----------------------
1. 修改recv接口返回53的问题
2. 修改串口接收丢数据的问题

V1.3.3修改记录
-----------------------
1. 修改socket close之后网络状态变回READY的问题
2. 修改network模块和TCPIP AT指令同时使用时，网络状态的问题
   修改后network和AT指令的网络(pdp)状态分开维护，互不影响
3. 网络状态增加了OPENAT_NETWORK_GOING_DOWN状态，表示网络(pdp)正在去激活状态
4. 增加mqtt应用

V1.4修改记录
-----------------------
1. 基础版本升级到V4719
2. 使用新的文件系统，改善flash擦写，文件保存的延迟
3. 增加SSL应用

V1.4.1修改记录
-----------------------
1. 增加getsockopt接口
2. 支持iot_flash_erase接口支持4KB对其擦除
3. 更新《Luat_IoT_SDK_C语言编程手册》

V1.4.2修改记录
-----------------------
1.基础版本升级到V4837
2. 修改多task socket select操作可能导致task被阻塞

V1.5修改记录
-----------------------
1. 基础版本升级到V5001
2. 更换编译器CSDTK4，兼容WIN10系统，从本版本开始，编译器必须使用CSDTK4

V1.5.1修改记录
-----------------------
1. 基础版本升级到V5021
2. 新增platfrom/IoT_SDK_UI项目
3. 支持iot_camera.h接口(仅基于platfrom/IoT_SDK_UI项目可用)
4. 新增iot_lcd.h接口(仅基于platfrom/IoT_SDK_UI项目可用)
5. 新增iot_touchscreen.h(仅基于platfrom/IoT_SDK_UI项目可用)，
6. 新增iot_keypad接口
7. 新增demo_ui
8. 新增demo_lcd
8. 新增demo_camera

V1.5.2
1. 升级基础版本V5135
2. 添加SPI 全双工不带CS功能
3. 添加UART波特率UART_BAUD_10400

V1.5.3
1. 升级基础版本到5163
2. 修改串口缓存buff分配失败, 导致串口初始化失败的问题

V1.5.4
1. 升级基础版本到5243
2. 修改摄像头不兼容问题,iot_camera_init接口添加了摄像头pin配置,摄像头分频设置, 摄像头SPI模式, 摄像头输出格式的设置