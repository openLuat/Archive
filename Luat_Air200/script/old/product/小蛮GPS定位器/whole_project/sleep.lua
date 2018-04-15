--[[
模块名称：gps休眠管理模块
模块功能：
模块最后修改时间：2017.02.09
]]

module(...,package.seeall)

local function print(...)
	_G.print("sleep",...)
end

--[[
函数名：itvwakesndfail
功能  ：定位数据12小时内都发送失败，重启设备
参数  ：无
返回值：无
]]
local function itvwakesndfail()
	nvm.set("abnormal",true)
	dbg.restart("itvwakesndfail")
end

--[[
函数名：itvwakesndsuc
功能  ：定位数据发送成功，关闭重启的定时器
参数  ：无
返回值：无
]]
local function itvwakesndsuc()
	print("itvwakesndsuc")
	sys.timer_stop(itvwakesndfail)
end

--[[
函数名：connsuc
功能  ：连接后台成功，开启数据发送失败则重启的定时器功能
参数  ：无
返回值：true
]]
local function connsuc()
	if not sys.timer_is_active(itvwakesndfail) then
		sys.timer_start(itvwakesndfail,43200000)
	end
	return true
end

--[[
函数名：wakegps
功能  ：唤醒gps
参数  ：无
返回值：无
]]
local function wakegps()
	print("wakegps",nvm.get("workmod"))
	nvm.set("gpsleep",false,"gps")
	sys.timer_stop(sys.dispatch,"ITV_GPSLEEP_REQ")
end

--[[
函数名：shkind
功能  ：震动消息处理
参数  ：无
返回值：true
]]
local function shkind()
	print("shkind",nvm.get("workmod"),nvm.get("gpsleep")) 
	if nvm.get("gpsleep") then
		sys.timer_start(sys.dispatch,_G.GPSMOD_WAKE_NOSHK_SLEEP_FREQ*1000,"ITV_GPSLEEP_REQ")
	else
		initgps()
	end	
	return true
end

--[[
函数名：parachangeind
功能  ：如果工作模式改变，重新开启gps
参数  ：k,v,r
返回值：true
]]
local function parachangeind(k,v,r)	
	print("parachangeind",k)
	if k == "workmod" then
		wakegps()
		initgps()
	end
	return true
end

--[[
函数名：gpsleep
功能  ：gps进入sleep模式，说明设备进入静止状态
参数  ：无
返回值：无
]]
local function gpsleep()
	print("gpsleep",nvm.get("workmod"))
	nvm.set("gpsleep",true,"gps")
end

--[[
函数名：itvgpslp
功能  ：进入gps休眠模式
参数  ：无
返回值：无
]]
local function itvgpslp()
	print("itvgpslp")
	gpsleep()
	sys.timer_stop(sys.dispatch,"ITV_GPSLEEP_REQ")
end

--[[
函数名：initgps
功能  ：为非gpsleep状态，如果5分钟内没检测到有效振动，进入休眠模式
        已为休眠模式，关闭进入休眠模式的定时器，关闭发送ITV_GPSLEEP_REQ的定时器
参数  ：无
返回值：true
]]
function initgps()
	print("initgps",nvm.get("workmod"),nvm.get("gpsleep"))	
	if not nvm.get("gpsleep") then
		sys.timer_start(gpsleep,_G.GPSMOD_CLOSE_SCK_INVALIDSHK_FREQ*1000)
	else
		sys.timer_stop(gpsleep)
		sys.timer_stop(sys.dispatch,"ITV_GPSLEEP_REQ")
	end	
	return true
end

--[[
函数名：gpsmodopnsck
功能  ：打开gps
参数  ：无
返回值：true
]]
local function gpsmodopnsck()
	print("gpsmodopnsck")
	initgps()
	wakegps()
	return true
end


local procer = {
	DEV_SHK_IND = shkind,
	GPSMOD_OPN_SCK_VALIDSHK_IND = gpsmodopnsck,
	PARA_CHANGED_IND = parachangeind,
	ITV_GPSLEEP_REQ = itvgpslp,
	ITV_WAKE_SNDSUC = itvwakesndsuc,
	LINKAIR_CONNECT_SUC = connsuc,
}

--注册app消息处理函数
sys.regapp(procer)
nvm.set("gpsleep",false)
initgps()
