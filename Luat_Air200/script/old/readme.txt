一个完整的项目脚本包含2部分：
1、第1部分是lib目录中的“库脚本”（所有项目都应该使用），
2、第2部分就是用户自己编写的“应用脚本”（例如demo和product目录下的脚本）

使用LuaDB工具烧写软件时，一定要选择这2部分脚本才能保证正常运行！！！


第1部分：
lib：“库脚本”，请注意：这个目录中的脚本是所有应用使用LuaDB工具下载时都需要包含得！！！


第2部分：
以下所有项目的“应用脚本”，demo目录下的lua脚本都可以在Air200开发板上运行，\product\小蛮GPS定位器下的lua脚本可以在小蛮GPS
定位器中运行。

由于时间有限，部分项目没有仔细测试，运行过程中可能出错，请自行验证，有问题QQ交流，谢谢！

在demo中：
audio：音频播放demo项目

call：语音通话demo项目

gpio\gpio_single：纯gpio控制demo项目

gpio\i2c_gpio_switch：i2c和gpio功能切换控制demo项目

gpio\uart_gpio_switch：uart和gpio功能切换控制demo项目

i2c：i2c通信demo项目

mqtt：mqtt应用demo项目

nvm：参数存储读写demo项目

pm：休眠控制demo项目

sms：短信demo项目

socket\long_connection：基于TCP的socket长连接通信demo项目（UDP使用方式和TCP完全相同）

socket\long_connection_transparent：基于TCP的socket透传demo项目，uart1透传数据到指定服务器

socket\short_connection：基于TCP的socket短连接通信demo项目（UDP使用方式和TCP完全相同）

socket\short_connection_flymode：基于TCP的socket短连接通信demo项目，会进入飞行模式并且定时退出飞行模式（UDP使用方式和TCP完全相同）

timer：定时器demo项目

uart：串口demo项目

uart_at_transparent：物理串口UART1透传AT命令demo项目，网络指示灯一直闪烁，亮100毫秒，灭2900毫秒（可通过开发板上的物理串口uart1，透传AT命令，波特率为115200，数据位8，停止位1，校验位和流控无；PC上的串口调试工具通过串口线和Air200开发板上的uart1相连，配置好串口参数，开发板上电开机，就可以支持AT命令的透传了）

uart_at_transparent_wdt：物理串口UART1透传AT命令demo项目（软件上支持硬件看门狗），网络指示灯一直闪烁，亮100毫秒，灭2900毫秒（可通过开发板上的物理串口uart1，透传AT命令，波特率为115200，数据位8，停止位1，校验位和流控无；PC上的串口调试工具通过串口线和Air200开发板上的uart1相连，配置好串口参数，开发板上电开机，就可以支持AT命令的透传了）

wdt：开启开发板上的硬件看门狗功能项目




\product\小蛮GPS定位器\whole_project：是合宙量产的一个定位器项目，有配套的后台以及app支持，app的名字为“时间线”。