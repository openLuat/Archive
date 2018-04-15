--必须在这个位置定义PROJECT和VERSION变量
--PROJECT：ascii string类型，可以随便定义，只要不使用,就行
--VERSION：ascii string类型，如果使用Luat物联云平台固件升级的功能，必须按照"X.X.X"定义，X表示1位数字；否则可随便定义
PROJECT = "Luat_TASK"
VERSION = "2.0.4"
-- 日志级别
require "log"
LOG_LEVEL = log.LOGLEVEL_TRACE
require "sys"
require "utils"
require "patch"
require "pins"
-- 加载GSM
require "net"
--8秒后查询第一次csq
net.startQueryAll(8 * 1000, 600 * 1000)
-- 控制台
require "console"
console.setup(1, 115200)
-- 系统工具
require "misc"
-- 看门狗
require "wdt"
wdt.setup(pio.P0_30, pio.P0_31)
-- 系統指示灯
require "ledTask"
-- AIR800的NET_LED(pio.P0_28)
-- AIR202的NET_LED(pio.P1_1)
-- 测试任务
require "testTask"
require "testMqtt"
-- require "testSocket"
-- require "testGPS"
require "ntp"
ntp.timeSync(1,function()log.info("----------------> AutoTimeSync is Done ! <----------------")end)
require "http"
require "audio"
-- require "i2cTest"
-- 测试LCD任务
-- require "ui"
-- ui.init(pio.P0_8, pio.P0_10, pio.P0_11, pio.P0_12)
-- require "mono_lcd_spi_sh1106"
-- mono_lcd_spi_sh1106.init(pio.P0_14, pio.P0_18, pio.P0_15, disp.BUS_SPI4LINE)
-- require "testLcd"
-- 启动系统框架
sys.init(0, 0)
sys.run()
