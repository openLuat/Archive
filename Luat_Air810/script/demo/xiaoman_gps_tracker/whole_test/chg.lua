require"sys"
module(...,package.seeall)

local inf = {}

local function print(...)
  _G.print("chg",...)
end

local function proc(msg)
	if msg then	
		print("proc",msg.charger,msg.state,msg.level,msg.voltage)
		if msg.level == 255 then return end
		setcharger(msg.charger)
		if inf.state ~= msg.state then
			inf.state = msg.state
			sys.dispatch("DEV_CHG_IND","CHG_STATUS",getstate())
		end
		
		inf.vol = msg.voltage
		inf.lev = msg.level
		local flag = (islowvolt() and getstate() ~= 1)
		if inf.low ~= flag then
			if (inf.low and (getstate()==1)) or flag then
				inf.low = flag
				sys.dispatch("DEV_CHG_IND","BAT_LOW",flag)
			end
			--[[inf.low = flag
			sys.dispatch("DEV_CHG_IND","BAT_LOW",flag)]]
		end		
		
		local flag = (islow1volt() and getstate() ~= 1)
		if inf.low1 ~= flag then
			if (inf.low1 and (getstate()==1)) or flag then
				inf.low1 = flag
				sys.dispatch("DEV_CHG_IND","BAT_LOW1",flag)
			end
		end	
		
		if inf.lev == 0 and not inf.chg then
			if not inf.poweroffing then
				inf.poweroffing = true
				sys.timer_start(sys.dispatch,30000,"REQ_PWOFF","BAT_LOW")
			end
		elseif inf.poweroffing then
			sys.timer_stop(sys.dispatch,"REQ_PWOFF","BAT_LOW")
			inf.poweroffing = false
		end
	end
end

local function init()	
	inf.vol = 3800
	inf.lev = 50
	inf.chg = nil
	inf.state = nil
	inf.poweroffing = false
	
	inf.lowvol = 3500
	inf.low = false
	inf.low1vol = 3600
	inf.low1 = false
end

function getcharger()
	return inf.chg
end

function setcharger(f)
	if inf.chg ~= f then
		inf.chg = f
		sys.dispatch("DEV_CHG_IND","CHARGER",f)
	end
end

function getvolt()
	return inf.vol
end

function getlev()
	if inf.lev == 255 then inf.lev = 95 end
	return inf.lev
end

--[[
函数名：getstate
功能  ：获取充电状态
参数  ：无
返回值：充电状态
		0 - 无充电器
		1 - 正在充电
		2 - 停止充电
]]
function getstate()
	return inf.state
end

function islow()
	return inf.low
end

function islow1()
	return inf.low1
end

function islowvolt()
	return inf.vol<=inf.lowvol
end

function islow1volt()
	return inf.vol<=inf.low1vol
end

sys.regmsg(rtos.MSG_PMD,proc)
init()
