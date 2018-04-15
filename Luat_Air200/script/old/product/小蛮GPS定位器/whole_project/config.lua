--[[
模块名称：配置文件
模块功能：配置各参数的默认值
模块最后修改时间：2017.02.13
]]

--定义一个模块，模块的名字与定义模块的文件的名字相同
module(...)

--配置参数的默认值
prot = "TCP"
addr = "lbsmqtt.airm2m.com"
port = "1884"
rptfreq = 1800
almfreq = 2
heart = 1800
guard = false
gpslbsmix = true
lbstyp = 2
fixmod = "GPS" --LBS,GPS,GPSWIFI
adminum = ""
lastworkmod = "GPS"
workmod = "GPS" --GPS,PWRGPS,SMS,LONGPS,PWOFF
workmodpend = false
gpsleep = false
led = true
lastlng = ""
lastlat = ""
callDmode = false
lngpsrptfreq = 30
pwrsleeptm = 3600
abnormal = true
