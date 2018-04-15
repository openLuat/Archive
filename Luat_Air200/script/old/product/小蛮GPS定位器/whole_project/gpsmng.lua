--[[
模块名称：gps应用管理模块
模块功能：管理gps的开启，关闭
模块最后修改时间：2017.02.13
]]
module(...,package.seeall)

--nogpslockflg,lowvoltlockflg暂时未用
--agpswrflg agps定位成功标记，true，agps已定位成功
local nogpslockflg,lowvoltlockflg,agpswrflg
--fstopn，true首次打开gps，nil或false非首次打开gps
local fstopn=true

local function print(...)
	_G.print("gpsmng",...)
end

--[[
函数名：opngps
功能  ：开启gps
参数  ：无
返回值：true
]]
local function opngps()
	print("opngps",nvm.get("workmod"),agpswrflg,chg.islow1(),chg.islow())
	if nvm.get("workmod")=="GPS" then
        gpsapp.open(gpsapp.TIMER,{cause="GPSMOD",val=_G.GPSMOD_CLOSE_GPS_INVALIDSHK_FREQ+5})
    end
    return true
end

--[[
函数名：fstopngps
功能  ：首次开启gps
参数  ：无
返回值：true
]]
local function fstopngps()
    if fstopn then 
        fstopn=false
        opngps()
        sys.dispatch("GPS_FST_OPN")
    end
    return true
end

--[[
函数名：agpswrsuc
功能  ：agps定位成功处理函数
参数  ：无
返回值：true
]]
local function agpswrsuc()
	print("syy agpswrsuc",agpswrflg,fstopn)
	if not agpswrflg and not fstopn then
		agpswrflg = true
		opngps()--fstopngps()
	end
	return true
end

--[[local function opnlongps()
	print("opnlongps")
	gpsapp.open(gpsapp.DEFAULT,{cause="LONGPSMOD"})
end

local function clslongps()
	print("clslongps")
	gpsapp.close(gpsapp.DEFAULT,{cause="LONGPSMOD"})
end]]

--[[
函数名：closegps
功能  ：关闭gps
参数  ：无
返回值：无
]]
function closegps()
	print("closegps")
	gpsapp.close(gpsapp.TIMER,{cause="GPSMOD"})
end

--[[local function workmodind(s)
	if nvm.get("workmod") ~= "GPS" then
		closegps()
		if nvm.get("workmod") == "LONGPS" then
			opnlongps()
		else
			clslongps()
		end
	else
		if s then opn() end
		clslongps()
	end
end

local function parachangeind(k,v,r)	
	if k == "workmod" then
		workmodind(true)
	end
	return true
end

local function init()	
	if nvm.get("workmod")=="GPS" then
		if rtos.poweron_reason() == rtos.POWERON_KEY or rtos.poweron_reason() == rtos.POWERON_CHARGER then
			opngps()
		end
	elseif nvm.get("workmod")~="LONGPS" then
		if chg.getcharger() then
			nvm.set("workmod","GPS","CHARGER")			
		end
	end
	workmodind()
end]]

--[[
函数名：gpstaind
功能  ：gps事件处理函数
参数  ：evt gps上报的事件
返回值：true
]]
local function gpstaind(evt)
	print("gpstaind",evt)
    --gps定位成功，分发定位成功的消息
	if evt == gps.GPS_LOCATION_SUC_EVT then
		sys.dispatch("GPS_FIX_SUC")
	end
	return true
end

--[[
函数名：shkind
功能  ：震动处理函数
参数  ：无
返回值：true
]]
local function shkind()
	if gpsapp.isactive(gpsapp.TIMER,{cause="GPSMOD"}) then
		opngps()
	end
	return true
end

--注册app消息处理函数
local procer =
{
	GPSMOD_OPN_GPS_VALIDSHK_IND = opngps,
	LINKAIR_CONNECT_SUC = fstopngps,
	--PARA_CHANGED_IND = parachangeind,
	[gps.GPS_STATE_IND] = gpstaind,
	DEV_SHK_IND = shkind,
	AGPS_WRDATE_SUC = agpswrsuc,
	MMI_KEYPAD_LONGPRESS_IND = opngps,
    MMI_KEYPAD_IND = opngps,
}

--sys.timer_start(agpswrsuc,180000)
sys.regapp(procer)
--init()
