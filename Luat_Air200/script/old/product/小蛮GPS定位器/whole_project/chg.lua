--[[
模块名称：电源模块
模块功能：充电参数配置，电池电量、充电状态消息处理
模块最后修改时间：2017.02.13
]]

--定义模块,导入依赖库
require"sys"
module(...,package.seeall)

local inf = {}
--电池百分值跟电池电量的对应表格
local tcap =
{
	[1] = {cap=100,volt=4200},
	[2] = {cap=90,volt=4060},
	[3] = {cap=80,volt=3980},
	[4] = {cap=70,volt=3920},
	[5] = {cap=60,volt=3870},
	[6] = {cap=50,volt=3820},
	[7] = {cap=40,volt=3790},
	[8] = {cap=30,volt=3770},
	[9] = {cap=20,volt=3740},
	[10] = {cap=10,volt=3680},
	[11] = {cap=5,volt=3500},
	[12] = {cap=0,volt=3400},
}

--[[
函数名：getcap
功能  ：根据电池电量换算成对应的百分比
参数  ：volt：电池电量
返回值：电池电量对应的百分比
]]
local function getcap(volt)
	if not volt then return 50 end
	if volt >= tcap[1].volt then return 100 end
	if volt <= tcap[#tcap].volt then return 0 end
	local idx,val,highidx,lowidx,highval,lowval = 0
	for idx=1,#tcap do
		if volt == tcap[idx].volt then
			return tcap[idx].cap
		elseif volt < tcap[idx].volt then
			highidx = idx
		else
			lowidx = idx
		end
		if highidx and lowidx then
			return (volt-tcap[lowidx].volt)*(tcap[highidx].cap-tcap[lowidx].cap)/(tcap[highidx].volt-tcap[lowidx].volt) + tcap[lowidx].cap
		end
	end
end

--[[
函数名：proc
功能  ：电源消息的处理
参数  ：msg：电池电量
返回值：无
]]
local function proc(msg)
	if msg then	
		print("chg proc",msg.charger,msg.state,msg.level,msg.voltage)
		if msg.level == 255 then return end
        --msg.charger false 充电器不在位，true充电器在位
		setcharger(msg.charger)
        --msg.state 0未连接充电器,1充电中，2已充满
		if inf.state ~= msg.state then
			inf.state = msg.state
			sys.dispatch("DEV_CHG_IND","CHG_STATUS",getstate())
		end
		inf.vol = msg.voltage
		inf.lev = getcap(msg.voltage)

        --flag：低电且不在充电中为true，否则为false
		local flag = (islowvolt() and getstate() ~= 1)
        --如果低电标记跟之前的低电标记值不同，则分发低电消息
		if inf.low ~= flag then
			if (inf.low and (getstate()==1)) or flag then
				inf.low = flag
				sys.dispatch("DEV_CHG_IND","BAT_LOW",flag)
			end
			--[[inf.low = flag
			sys.dispatch("DEV_CHG_IND","BAT_LOW",flag)]]
		end		
		--flag：低电1且不在充电中为true，否则为false
		local flag = (islow1volt() and getstate() ~= 1)
        --如果低电标记1跟之前的低电标记1值不同，则分发低电1消息
		if inf.low1 ~= flag then
			if (inf.low1 and (getstate()==1)) or flag then
				inf.low1 = flag
				sys.dispatch("DEV_CHG_IND","BAT_LOW1",flag)
			end
		end	
		
        --如果电量等级为0且充电器不在位，则发送关机请求，否则关掉发送关机请求的定时器
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

--[[
函数名：init
功能  ：充电参数配置，初始化
参数  ：无
返回值：无
]]
local function init()	
	inf.vol = 3800
	inf.lev = 50
	inf.chg = false
	inf.state = false
	inf.poweroffing = false
	
	inf.lowvol = 3500
	inf.lowlev = 10
	inf.low = false
	inf.low1vol = _G.LOWVOLT_FLY
	inf.low1 = false
	
	local para = {}
	para.batdetectEnable = 0--pmd电池在位检测
	para.currentFirst = 200--设置第一阶段充电电流
	para.currentSecond = 100--设置第二阶段充电电流
	para.currentThird = 50--设置第三阶段充电电流
	para.intervaltimeFirst = 180--设置第一阶段充电超时时间单位秒
	para.intervaltimeSecond = 60--设置第二阶段充电超时时间单位秒
	para.intervaltimeThird = 30--设置第三阶段充电超时时间单位秒
	para.battlevelFirst = 4100--未使用
	para.battlevelSecond = 4150--未使用
	para.pluschgctlEnable = 1--脉冲充电控制
	para.pluschgonTime = 5--脉冲充电时间，单位秒
	para.pluschgoffTime = 1--脉冲停止冲电时间，单位秒
	pmd.init(para)
end

--[[
函数名：getcharger
功能  ：获取充电器在位情况
参数  ：无
返回值：false 充电器不在位，true充电器在位
]]
function getcharger()
	return inf.chg
end

--[[
函数名：setcharger
功能  ：设置充电器在位情况
参数  ：f，false 充电器不在位，true充电器在位
返回值：无
]]
function setcharger(f)
	if inf.chg ~= f then
		inf.chg = f
		sys.dispatch("DEV_CHG_IND","CHARGER",f)
	end
end

--[[
函数名：getvolt
功能  ：获取电池电量
参数  ：无
返回值：电池电量值
]]
function getvolt()
	return inf.vol
end

--[[
函数名：getlev
功能  ：获取电池电量等级
参数  ：无
返回值：电池电量等级值
]]
function getlev()
	if inf.lev == 255 then inf.lev = 95 end
	return inf.lev
end

--[[
函数名：getstate
功能  ：获取电池充电状态
参数  ：无
返回值：0，未充电，1正在充电，2已充满
]]
function getstate()
	return inf.state
end

--[[
函数名：islow
功能  ：判断是否低电
参数  ：无
返回值：true，低电，false，不是低电
]]
function islow()
	return inf.low
end

--[[
函数名：islow1
功能  ：判断是否低电1
参数  ：无
返回值：true，低电，false，不是低电
]]
function islow1()
	return inf.low1
end

--[[
函数名：islowvolt
功能  ：判断电量是否低于电量lowvol
参数  ：无
返回值：true，是，false，否
]]
function islowvolt()
	return inf.vol<=inf.lowvol
end

--[[
函数名：islow1volt
功能  ：判断电量是否低于电量lowvol1
参数  ：无
返回值：true，是，false，否
]]
function islow1volt()
	return inf.vol<=inf.low1vol
end

--注册电源消息处理函数
sys.regmsg(rtos.MSG_PMD,proc)
--初始化充电配置
init()
