--[[
模块名称：gps应用模块
模块功能：控制gps的开启关闭操作
模块最后修改时间：2017.02.13
]]

module(...,package.seeall)
--DEFAULT：默认开启模式，由开启方自行控制何时关闭；定位成功后会延时10秒钟执行回调
--TIMERORSUC：定时器开启模式1，不需要开启方控制何时关闭；定位成功延时10秒或者定时器时间到后,会执行回调；然后关闭GPS
--TIMER：定时器开启模式2，不需要开启方控制何时关闭；定时器时间到后,如果定位成功延时10秒会执行回调，如果失败，立即执行回调；然后关闭GPS
DEFAULT,TIMERORSUC,TIMER = 0,1,2
--tlist记录开启GPS的每个需求项，只有所有需求项都执行了关闭动作，才真正关闭GPS
local tlist,flg = {}
local DELAY_CTL,OPEN_MAX_TIME = true,300

--[[
函数名：delitem
功能  ：删除某项记录
参数  ：mode，gps打开模式
        para，参数
返回值：无
]]
local function delitem(mode,para)
	local i
	for i=1,#tlist do
		if tlist[i].flag and tlist[i].mode == mode and tlist[i].para.cause == para.cause then
			tlist[i].flag,tlist[i].delay = false
			break
		end
	end
end

--[[
函数名：additem
功能  ：添加某项记录
参数  ：mode，gps打开模式
        para，参数
返回值：无
]]
local function additem(mode,para)
	delitem(mode,para)
	local item,i,fnd = {flag = true, mode = mode, para = para}
	if mode == TIMERORSUC or mode == TIMER then item.para.remain = para.val end
	for i=1,#tlist do
		if not tlist[i].flag then
			tlist[i] = item
			fnd = true
			break
		end
	end
	if not fnd then table.insert(tlist,item) end
end

--[[
函数名：isexisttimeritem
功能  ：是否存在定时开启模式项
参数  ：无
返回值：true存在，nil不存在
]]
local function isexisttimeritem()
	local i
	for i=1,#tlist do
		if tlist[i].flag and (tlist[i].mode == TIMERORSUC or tlist[i].mode == TIMER or tlist[i].para.delay) then return true end
	end
end

--[[
函数名：timerfunc
功能  ：定时函数，每秒查询一次对应的item是否定位成功，成功删除对应的item，
        如果时间到，item是仍未定位成功，也删除对应的item
参数  ：无
返回值：无
]]
local function timerfunc()
	local i
	for i=1,#tlist do
		print("gpsapp.timerfunc@"..i,tlist[i].flag,tlist[i].mode,tlist[i].para.cause,tlist[i].para.val,tlist[i].para.remain,tlist[i].para.delay,tlist[i].para.cb)
		if tlist[i].flag then
			local rmn,dly,md,cb = tlist[i].para.remain,tlist[i].para.delay,tlist[i].mode,tlist[i].para.cb
			if rmn and rmn > 0 then
				tlist[i].para.remain = rmn - 1
			end
			if dly and dly > 0 then
				tlist[i].para.delay = dly - 1
			end
			
			rmn = tlist[i].para.remain
			if gps.isfix() and md == TIMER and rmn == 0 and not tlist[i].para.delay then
				tlist[i].para.delay = 10
			end
			
			dly = tlist[i].para.delay
			if gps.isfix() then
				if dly and dly == 0 then
					if cb then cb(tlist[i].para.cause) end
					if md == DEFAULT then
						tlist[i].para.delay = nil
					else
						close(md,tlist[i].para)
					end
				end
			else
				if rmn and rmn == 0 then
					local val,sn,satecnt,delay = tlist[i].para.val,gps.getgpssn(),gps.getpara().gsasatecnt
					print("gpsapp.delayctl",val,sn,satecnt)
					if DELAY_CTL and val and val < OPEN_MAX_TIME and not tlist[i].para.phase then
						if sn >= 30 and (val < 120 or satecnt >= 3) then
							delay = true
							local para = tlist[i].para
							val = OPEN_MAX_TIME - val
							open(DEFAULT,{cause="GPSAPPTEST"})
							if tlist[i].para.imcb and cb then cb(tlist[i].para.cause) end
							close(md,tlist[i].para)
							para.val = val
							para.phase = true
							open(md,para)
							close(DEFAULT,{cause="GPSAPPTEST"})
						end
					end
					if not delay then
						if cb then cb(tlist[i].para.cause) end
						close(md,tlist[i].para)
					end					
				end
			end			
		end
	end
	if isexisttimeritem() then sys.timer_start(timerfunc,1000) end
end

--[[
函数名：gpsstatind
功能  ：gps事件处理函数
参数  ：id,evt
返回值：true
]]
local function gpsstatind(id,evt)
    print("gpsapp-gpsstatind evt",evt)
	if evt == gps.GPS_LOCATION_SUC_EVT then
		local i
		for i=1,#tlist do
			print("gpsapp.gpsstatind@"..i,tlist[i].flag,tlist[i].mode,tlist[i].para.cause,tlist[i].para.val,tlist[i].para.remain,tlist[i].para.delay,tlist[i].para.cb)
			if tlist[i].flag then
				if tlist[i].mode ~= TIMER then
					tlist[i].para.delay = 10
					if tlist[i].mode == DEFAULT then
						if isexisttimeritem() then sys.timer_start(timerfunc,1000) end
					end
				end				
			end			
		end
	elseif evt == gps.GPS_CLOSE_EVT then
		flg = nil
	end
	return true
end

function updfixmode()
end

--[[
函数名：forceclose
功能  ：强制关掉gps
参数  ：无
返回值：无
]]
function forceclose()
	local i
	for i=1,#tlist do
		if tlist[i].flag and tlist[i].para.cb then tlist[i].para.cb(tlist[i].para.cause) end
		close(tlist[i].mode,tlist[i].para)
	end
end

--[[
函数名：close
功能  ：只有所有需求项都执行了关闭动作，才真正关闭GPS
参数  ：mode,para
返回值：无
]]
function close(mode,para)
	assert((para and _G.type(para) == "table" and para.cause),"gpsapp.close para invalid")
	print("gpsapp.ctl close",mode,para.cause,para.val,para.cb)
	delitem(mode,para)
	local valid,i
	for i=1,#tlist do
		if tlist[i].flag then
			valid = true
		end		
	end
	if not valid then gps.closegps("gpsapp") end
end

--[[
函数名：open
功能  ：开启gps
参数  ：mode,para
返回值：无
]]
function open(mode,para)
	assert((para and _G.type(para) == "table" and para.cause),"gpsapp.open para invalid")
	print("gpsapp.ctl open",mode,para.cause,para.val,para.cb)
	if gps.isfix() then
		if mode ~= TIMER then
			if para.cb then para.cb(para.cause) end
			if mode == TIMERORSUC then return end			
		end
	end
	additem(mode,para)
	gps.opengps("gpsapp")
	updfixmode()	
	if isexisttimeritem() and not sys.timer_is_active(timerfunc) then
		sys.timer_start(timerfunc,1000)
	end
end

--[[
函数名：isactive
功能  ：判断某需求项是否为激活状态
参数  ：mode,para
返回值：true为激活状态否则为非激活状态
]]
function isactive(mode,para)
	local i
	for i=1,#tlist do
		if tlist[i].flag and tlist[i].mode == mode and tlist[i].para.cause == para.cause then
			return true
		end
	end
end

--初始化gps
gps.initgps(nil,nil,true,1000,2,9600,8,uart.PAR_NONE,uart.STOP_1)
gps.setgpsfilter(5)
update.settimezone(update.BEIJING_TIME)
gps.setspdtyp(gps.GPS_KILOMETER_SPD)
--注册gps上报的消息 的处理函数
sys.regapp(gpsstatind,gps.GPS_STATE_IND)
