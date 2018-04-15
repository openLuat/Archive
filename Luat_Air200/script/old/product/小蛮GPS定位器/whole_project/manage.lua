--[[
模块名称：manage
模块功能：管理设备的运动状态的切换，SIM卡工作情况等的一些接口
模块最后修改时间：2017.02.13
]]
module(...,package.seeall)

GUARDFNC,HANDLEPWRFNC,MOTORPWRFNC,RMTPWRFNC,BUZZERFNC = true,false,false,false,false

local lastyp,lastlng,lastlat,lastmlac,lastmci,lastlbs2 = "","","","","",""
local sta = "MOV"

--[[
函数名：setlastgps
功能  ：设置上一次gps的经纬度
参数  ：lng 上一次gps的经度
		lat：上一次gps的纬度
返回值：无
]]
function setlastgps(lng,lat)
	lastyp,lastlng,lastlat = "GPS",lng,lat
	nvm.set("lastlng",lng,nil,false)
	nvm.set("lastlat",lat)
end

--[[
函数名：getlastgps
功能  ：获取上一次gps的经纬度
参数  ：无
返回值：上一次gps的经度，上一次gps的纬度
]]
function getlastgps()
	return nvm.get("lastlng"),nvm.get("lastlat")
end

--[[
函数名：isgpsmove
功能  ：判断gps是否在移动，此函数暂未用到，为空函数
参数  ：lng,lat
返回值：true
]]
function isgpsmove(lng,lat)
	--[[if nvm.get("workmod") ~= "GPS" then return true end
	if lastlng=="" or lastlat=="" or lastyp~="GPS" then return true end
	local dist = gps.diffofloc(lat,lng,lastlat,lastlng)
	print("isgpsmove",lat,lng,lastlat,lastlng,dist)
	return dist >= 15*15 or dist < 0]]
	return true
end

--[[
函数名：setlastlbs1
功能  ：设置上一次基站信息
参数  ：lac,ci,flg
返回值：无
]]
function setlastlbs1(lac,ci,flg)
	lastmlac,lastmci = lac,ci
	if flg then lastyp = "LBS1" end
end

--[[
函数名：islbs1move
功能  ：此函数暂未用到，为空函数
参数  ：lac,ci
返回值：true
]]
function islbs1move(lac,ci)
	--[[if nvm.get("workmod") ~= "GPS" then return true end
	return lac ~= lastmlac or ci ~= lastmci]]
	return true
end

--[[
函数名：setlastlbs2
功能  ：设置上一次基站信息
参数  ：v,flg
返回值：无
]]
function setlastlbs2(v,flg)
	lastlbs2 = v
	if flg then lastyp = "LBS2" end
end

--[[
函数名：islbs2move
功能  ：此函数暂未用到，为空函数
参数  ：v
返回值：true
]]
function islbs2move(v)
	--[[if nvm.get("workmod") ~= "GPS" then return true end
	if lastlbs2 == "" then return true end
	local oldcnt,newcnt,subcnt,chngcnt,laci = 0,0,0,0
	
	for laci in string.gmatch(lastlbs2,"(%d+%.%d+%.%d+%.%d+%.)%d+;") do
		oldcnt = oldcnt + 1
	end
	
	for laci in string.gmatch(v,"(%d+%.%d+%.%d+%.%d+%.)%d+;") do
		newcnt = newcnt + 1
		if not string.match(lastlbs2,laci) then chngcnt = chngcnt + 1 end
	end
	
	if oldcnt > newcnt then chngcnt = chngcnt + (oldcnt-newcnt) end
	local move = chngcnt*100/(newcnt>oldcnt and newcnt or oldcnt)
	print("islbs2move",lastlbs2,v,move)
	return move >= 50]]
	return true
end

--[[
函数名：getlastyp
功能  ：获取上一次定位类型
参数  ：无
返回值：上一次定位类型
]]
function getlastyp()
	return lastyp
end

--[[
函数名：resetlastloc
功能  ：重置上一次位置信息
参数  ：无
返回值：无
]]
function resetlastloc()
	lastyp,lastlng,lastlat,lastmlac,lastmci,lastlbs2 = "","","","","",""
end

--[[
函数名：chgind
功能  ：DEV_CHG_IND消息处理函数
参数  ：evt 电池消息事件
        val true或false
返回值：true
]]
local function chgind(evt,val)
	print("manage chgind",nvm.get("workmod"),evt,val)
    --如果是低电事件，则发送关机请求
    if evt == "BAT_LOW" and val then
        sys.dispatch("REQ_PWOFF","BAT_LOW")
    end
	return true
end

--[[
函数名：handle_silsta
功能  ：设备静止状态的处理
参数  ：无
返回值：true
]]
local function handle_silsta()
    print("manage handle_silsta ",sta)
    --如果之前是运动状态，则分发状态改变请求，告诉设备已切到静止状态
    if sta ~= "SIL" then
        sta = "SIL"
        sys.dispatch("STA_CHANGE",sta)
    end
    return true
end

--[[
函数名：handle_movsta
功能  ：设备运动状态的处理
参数  ：无
返回值：true
]]
local function handle_movsta()
    print("manage handle_movsta ",sta)
    --如果之前是静止状态，则分发状态改变请求，告诉设备已切到运动状态
    if sta ~= "MOV" then
        sta = "MOV"
        sys.dispatch("STA_CHANGE",sta)
        sys.timer_start(handle_silsta,_G.STA_SIL_VALIDSHK_CNT*_G.STA_SIL_VALIDSHK_FREQ*1000)
    end
    return true
end

--[[
函数名：handle_movsta
功能  ：STA_CHANGE消息的处理函数，暂未用，实为空函数
参数  ：sta
返回值：true
]]
local function sta_change(sta)
    local mod = nvm.get("workmod")
    print("manage sta_change mod",mod,sta)
    --workmodind()
    return true
end

--[[
函数名：shkind
功能  ：震动消息的处理函数
参数  ：无
返回值：true
]]
local function shkind()
    sys.timer_stop(handle_silsta)
    sys.timer_start(handle_silsta,_G.STA_SIL_VALIDSHK_CNT*_G.STA_SIL_VALIDSHK_FREQ*1000)
    return true
end

--[[
函数名：getmovsta
功能  ：获取设备的运动状态
参数  ：无
返回值："MOV"运动，"SIL"静止
]]
function getmovsta()
    return sta
end

local hassim=true
--[[
函数名：simind
功能  ：SIM_IND消息处理函数
参数  ：para，上报的消息参数
返回值：true
]]
local function simind(para)
	print("simind p",para)
    --SIM卡不存在
	if para == "NIST" then
        if hassim then
            nvm.set("abnormal",true)
            sys.timer_start(sys.restart,300000,"power on without sim")
        end
        hassim=false
    --SIM卡准备就绪
    elseif para == "RDY" then
        hassim=true
        sys.timer_stop(sys.restart,"power on without sim")
    end

	return true
end

--[[
函数名：issimexist
功能  ：查询SIM卡是否存在
参数  ：无
返回值：true，存在，其他值，不存在
]]
function issimexist()
	return hassim
end

local procer =
{
	DEV_CHG_IND = chgind,
	SIM_IND = simind,
	DEV_SHK_IND = shkind,
    STA_MOV_VALIDSHK_IND = handle_movsta,
    --STA_CHANGE = sta_change,
}

--注册app消息处理函数
sys.regapp(procer)
sys.timer_start(handle_silsta,_G.STA_SIL_VALIDSHK_CNT*_G.STA_SIL_VALIDSHK_FREQ*1000)
