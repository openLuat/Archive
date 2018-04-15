module(...,package.seeall)

require"misc"

--将要写入设备的新imei号,15位
local newimei = "123456789012347"

--5秒后开始写imei
sys.timer_start(misc.setimei,5000,newimei)
