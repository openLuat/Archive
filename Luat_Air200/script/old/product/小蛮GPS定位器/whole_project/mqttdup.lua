module(...,package.seeall)

local DUP_TIME,DUP_CNT,tlist = 10,3,{}

local function print(...)
	_G.print("mqttdup",...)
end

--[[
函数名：timerfnc
功能  ：定时器函数，dup剩余时间为0时发送MQTT_DUP_IND消息
参数  ：无
返回值：无
]]
local function timerfnc()
	print("timerfnc")
	for i=1,#tlist do
		print(i,tlist[i].tm)
		if tlist[i].tm > 0 then
			tlist[i].tm = tlist[i].tm-1
			if tlist[i].tm == 0 then
				sys.dispatch("MQTT_DUP_IND",tlist[i].dat)
			end
		end
	end
end

--[[
函数名：timer
功能  ：启动或关闭定时器函数
参数  ：start true：启动，其他值关闭
返回值：无
]]
local function timer(start)
	print("timer",start)
	if start then
		if not sys.timer_is_active(timerfnc) then
			sys.timer_loop_start(timerfnc,1000)
		end
	else
		if #tlist == 0 then sys.timer_stop(timerfnc) end
	end
end

--[[
函数名：ins
功能  ：增加某个列表项
参数  ：typ,dat,seq
返回值：无
]]
function ins(typ,dat,seq)
	print("ins",typ,common.binstohexs(dat),seq or "nil" or common.binstohex(seq))
	table.insert(tlist,{typ=typ,dat=dat,seq=seq,cnt=DUP_CNT,tm=DUP_TIME})
	timer(true)
end

--[[
函数名：rmv
功能  ：删除某个列表项
参数  ：typ,dat,seq
返回值：无
]]
function rmv(typ,dat,seq)
	print("rmv",typ or getyp(seq),common.binstohexs(dat),seq or "nil" or common.binstohex(seq))
	for i=1,#tlist do
		if (not typ or typ == tlist[i].typ) and (not dat or dat == tlist[i].dat) and (not seq or seq == tlist[i].seq) then
			table.remove(tlist,i)
			break
		end
	end
	timer()
end

--[[
函数名：rmvall
功能  ：删除所有列表项
参数  ：无
返回值：无
]]
function rmvall()
	tlist = {}
	timer()
end

--[[
函数名：rsm
功能  ：dup次数减一，如果dup剩余次数大于0，dup时间回复默认是DUP_TIME，如果dup剩余次数为0，删除此列表项
参数  ：seq序列号
返回值：无
]]
function rsm(s)
	for i=1,#tlist do
		if tlist[i].dat == s then
			tlist[i].cnt = tlist[i].cnt - 1
			if tlist[i].cnt == 0 then rmv(nil,s) return end
			tlist[i].tm = DUP_TIME			
			break
		end
	end
end

--[[
函数名：getyp
功能  ：通过序列号得知列表项的消息类型
参数  ：seq序列号
返回值：无
]]
function getyp(seq)
	for i=1,#tlist do
		if seq and seq == tlist[i].seq then
			return tlist[i].typ
		end
	end
end
