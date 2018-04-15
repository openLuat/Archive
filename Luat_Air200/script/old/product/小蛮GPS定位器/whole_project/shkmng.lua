--[[
模块名称：震动管理模块
模块功能：震动消息的管理
模块最后修改时间：2017.02.09
]]

module(...,package.seeall)

local function print(...)
	_G.print("shkmng",...)
end

local tick = 0

--[[
函数名：timerfnc
功能  ：定时计时函数，每秒tick加1
参数  ：无
返回值：无
]]
local function timerfnc()
	tick = tick+1
end

local tshkapp = 
{
	["GPSMOD_OPN_SCK"] = {flg={},idx=0,cnt=_G.GPSMOD_OPN_SCK_VALIDSHK_CNT,freq=_G.GPSMOD_OPN_SCK_VALIDSHK_FREQ},
	--["GPSMOD_LOWVOLT_OPN_SCK"] = {flg={},idx=0,cnt=_G.GPSMOD_LOWVOLT_OPN_SCK_VALIDSHK_CNT,freq=_G.GPSMOD_LOWVOLT_OPN_SCK_VALIDSHK_FREQ},
	--["GPSMOD_NOGPS_OPN_SCK"] = {flg={},idx=0,cnt=_G.GPSMOD_NOGPS_OPN_SCK_VALIDSHK_CNT,freq=_G.GPSMOD_NOGPS_OPN_SCK_VALIDSHK_FREQ},
	["GPSMOD_OPN_GPS"] = {flg={},idx=0,cnt=_G.GPSMOD_OPN_GPS_VALIDSHK_CNT,freq=_G.GPSMOD_OPN_GPS_VALIDSHK_FREQ},
	--["GPSMOD_LOWVOLT_OPN_GPS"] = {flg={},idx=0,cnt=_G.GPSMOD_LOWVOLT_OPN_GPS_VALIDSHK_CNT,freq=_G.GPSMOD_LOWVOLT_OPN_GPS_VALIDSHK_FREQ},
	--["GPSMOD_NOGPS_OPN_GPS"] = {flg={},idx=0,cnt=_G.GPSMOD_NOGPS_OPN_GPS_VALIDSHK_CNT,freq=_G.GPSMOD_NOGPS_OPN_GPS_VALIDSHK_FREQ},
	--["PWRMOD_OPN_SCK"] = {flg={},idx=0,cnt=_G.PWRMOD_OPN_SCK_VALIDSHK_CNT,freq=_G.PWRMOD_OPN_SCK_VALIDSHK_FREQ},
	["LONGPSMOD"] = {flg={},idx=0,cnt=_G.LONGPSMOD_VALIDSHK_CNT,freq=_G.LONGPSMOD_VALIDSHK_FREQ},
	["STA_MOV"] = {flg={},idx=0,cnt=_G.STA_MOV_VALIDSHK_CNT,freq=_G.STA_MOV_VALIDSHK_FREQ},
}

--[[
函数名：reset
功能  ：重置shkapp参数
参数  ：name
返回值：无
]]
local function reset(name)
	local i
	for i=1,tshkapp[name].cnt do
		tshkapp[name].flg[i] = 0
	end
	tshkapp[name].idx = 0
end

local function shkprint(name,suffix)
	local str,i = ""	
	for i=1,tshkapp[name].cnt do
		str = str..","..tshkapp[name].flg[i]
	end
	print(name..suffix,str)
end

--[[
函数名：fnc
功能  ：shkapp对震动消息的处理，连续个指定的freq中都有震动，则分发对应的震动消息
参数  ：无
返回值：无
]]
local function fnc()
	local k,v
	for k,v in pairs(tshkapp) do
		shkprint(k,"1")
		if v.idx==0 then
			v.flg[1] = tick
			v.idx = 1
		elseif v.idx<v.cnt then
            --连续个指定的freq中都有震动，则分发对应的震动消息
			if ((tick-v.flg[v.idx])>v.freq) and ((tick-v.flg[v.idx])<(v.freq*2)) then
				v.idx = v.idx+1
				if v.idx==v.cnt then
					v.idx = 1
					v.flg[v.cnt-1] = tick
					sys.dispatch(k.."_VALIDSHK_IND")
					print(k.."_VALIDSHK_IND")
				else
					v.flg[v.idx] = tick
				end
			elseif (tick-v.flg[v.idx])>=(v.freq*2) then
				reset(k)
			end
		end
		shkprint(k,"2")
	end	
end

--[[
函数名：shkind
功能  ：震动消息处理函数
参数  ：无
返回值：true
]]
local function shkind()
	fnc()
	return true
end

--[[
函数名：init
功能  ：初始化震动管理app
参数  ：无
返回值：true
]]
local function init()
	local k,v
	for k,v in pairs(tshkapp) do
		reset(k)
	end
end

init()
sys.regapp(shkind,"DEV_SHK_IND")
sys.timer_loop_start(timerfnc,1000)
