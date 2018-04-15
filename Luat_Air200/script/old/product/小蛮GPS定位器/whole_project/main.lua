--[[
模块名称：主程序
模块功能：程序入口
模块最后修改时间：2017.02.13
]]

--项目名
PROJECT = "A5315S"
--项目id
PRJID=21
--版本号
VERSION = "1.0.0"
--远程升级模式 0，自动  1，收动
UPDMODE = 0
--修改时间
MDFYTIME = "201608261317"
--全接变量参数值
LONGPSMOD_DFT_HEART = 1800
LONGPSMOD_DFT_RPTFREQ = 60
LONGPSMOD_SHK_HEART = 1800
--LONGPSMOD_SHK_RPTFREQ = 30
LONGPSMOD_VALIDSHK_CNT = 3
LONGPSMOD_VALIDSHK_FREQ = 10
GPSMOD_DFT_HEART = 900
GPSMOD_DFT_RPTFREQ = 60
GPSMOD_SIL_HEART = 3000
GPSMOD_SIL_RPTFREQ = 3600*8
GPSMOD_OPN_SCK_VALIDSHK_CNT = 3
GPSMOD_OPN_SCK_VALIDSHK_FREQ = 10
--GPSMOD_LOWVOLT_OPN_SCK_VALIDSHK_CNT = 3
--GPSMOD_LOWVOLT_OPN_SCK_VALIDSHK_FREQ = 10
--GPSMOD_NOGPS_OPN_SCK_VALIDSHK_CNT = 3
--GPSMOD_NOGPS_OPN_SCK_VALIDSHK_FREQ = 10
GPSMOD_OPN_GPS_VALIDSHK_CNT = 3
GPSMOD_OPN_GPS_VALIDSHK_FREQ = 10
GPSMOD_CLOSE_GPS_INVALIDSHK_FREQ = 60
GPSMOD_NOSHK_WAKE_HEART = 900
GPSMOD_NOSHK_WAKE_FREQ = 3600*2
GPSMOD_WAKE_NOSHK_SLEEP_FREQ = 300
GPSMOD_LOWVOLT_OPN_GPS_VALIDSHK_CNT = 3
GPSMOD_LOWVOLT_OPN_GPS_VALIDSHK_FREQ = 10
GPSMOD_CLOSE_SCK_INVALIDSHK_FREQ = 300
PWRMOD_DFT_HEART = 7200
PWRMOD_DFT_RPTFREQ = 1200
PWRMOD_NOSLEEP_HEART = 3600
PWRMOD_NOSLEEP_RPTFREQ = 3600
--PWRMOD_SIL_HEART = 3600
--PWRMOD_SIL_RPTFREQ = 3600
PWOFFMOD_DFT_HEART = 43200
PWOFFMOD_DFT_RPTFREQ = 43200
PWRMOD_OPN_SCK_VALIDSHK_CNT = 2
PWRMOD_OPN_SCK_VALIDSHK_FREQ = 10
PWRMOD_CLOSE_SCK_INVALIDSHK_FREQ = 300
PWRMOD_NOSHK_WAKE_FREQ = 3600*4
PWRMOD_WAKE_NOSHK_SLEEP_FREQ = 300
GPSMOD_GPS_FAIL_TIME = 720
GPSMOD_LOWVOLT_LOCK_GPS_TIME = 3600*2
GPSMOD_NOGPS_LOCK_GPS_TIME = 3600*2
LOWVOLT_WAKE_FREQ = 3600*8
LOWVOLT_SLEEP_FREQ = 90
LOWVOLT_FLY = 3600 --单位MV
CLOSE_SCK_INVALIDSHK_FREQ = 300
STA_MOV_VALIDSHK_CNT = 3
STA_MOV_VALIDSHK_FREQ = 10
STA_SIL_VALIDSHK_CNT = 5
STA_SIL_VALIDSHK_FREQ = 60
_G.collectgarbage("setpause",90)
--[[
函数名：appendprj
功能  ：给工程名添加尾缀
参数  ：suffix
返回值：无
]]
function appendprj(suffix)
	PROJECT = PROJECT .. suffix
end
--导入依赖库
require"sys"
require"nvm"
require"pins"
require"chg"
require"audio"
require"light"
require"link"
require"update"
require"gps"
require"dbg"
require"shk"
require"shkmng"
require"agps"
require"gpsapp"
require"gpsmng"
require"manage"
require"linkapp"
require"mqtt"
require"linkair"
require"keypad"
require"smsapp"
require"wdt153b"
require"factory"
require"sleep"
require"rcd"

local apntable =
{
	["46000"] = "CMNET",
	["46002"] = "CMNET",
	["46004"] = "CMNET",
	["46007"] = "CMNET",
	["46001"] = "UNINET",
	["46006"] = "UNINET",
}

--[[
函数名：proc
功能  ：设置apn名
参数  ：id，app消息id
返回值：true
]]
local function proc(id)
	link.setapn(apntable[sim.getmcc()..sim.getmnc()] or "CMNET")
	return true
end

--注册IMSI_READY消息处理函数
sys.regapp(proc,"IMSI_READY")
--初始化程序
sys.init(0,0)
--ril.request("AT*TRACE=\"SXS\",1,0")
--ril.request("AT*TRACE=\"DSS\",1,0")
--ril.request("AT*TRACE=\"RDA\",1,0")
--运行程序
sys.run()
