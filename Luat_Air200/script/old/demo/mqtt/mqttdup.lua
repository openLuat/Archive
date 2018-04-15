--[[
模块名称：publish报文重发管理
模块功能：QoS为1的publish报文重发处理
          发送publish报文后，如果DUP_TIME秒内没收到puback，则会自动重发，最多重发DUP_CNT次，如果都没收到puback，则不再重发，抛出MQTT_DUP_FAIL消息，然后丢弃该报文
模块最后修改时间：2017.02.24
]]

module(...,package.seeall)

--DUP_TIME：发送publish报文后，DUP_TIME秒内判断有没有收到puback
--DUP_CNT：没有收到puback报文的publish报文重发的最大次数
--tlist：publish报文存储表
local DUP_TIME,DUP_CNT,tlist = 10,3,{}
local slen = string.len

--[[
函数名：print
功能  ：打印接口，此文件中的所有打印都会加上mqttdup前缀
参数  ：无
返回值：无
]]
local function print(...)
	_G.print("mqttdup",...)
end

--[[
函数名：timerfnc
功能  ：1秒的定时器处理函数，查询tlist中的publish报文是否到时间需要重发
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
功能  ：开启或者关闭1秒的定时器
参数  ：
		start：开启或者关闭，true开启，false或者nil关闭
返回值：无
]]
local function timer(start)
	print("timer",start,#tlist)
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
功能  ：插入一条publish报文到存储表
参数  ：
		typ：用于自定义类型
		dat：publish报文数据
		seq：publish报文序列号
返回值：无
]]
function ins(typ,dat,seq)
	print("ins",typ,(slen(dat or "") > 200) and "" or common.binstohexs(dat),seq or "nil" or common.binstohex(seq))
	table.insert(tlist,{typ=typ,dat=dat,seq=seq,cnt=DUP_CNT,tm=DUP_TIME})
	timer(true)
end

--[[
函数名：rmv
功能  ：从存储表删除一条publish报文
参数  ：
		typ：用于自定义类型
		dat：publish报文数据
		seq：publish报文序列号
返回值：无
]]
function rmv(typ,dat,seq)
	print("rmv",typ or getyp(seq),(slen(dat or "") > 200) and "" or common.binstohexs(dat),seq or "nil" or common.binstohex(seq))
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
功能  ：从存储表删除所有publish报文
参数  ：无
返回值：无
]]
function rmvall()
	tlist = {}
	timer()
end

--[[
函数名：rsm
功能  ：重发一条publish报文后的回调处理
参数  ：
		s：publish报文数据
返回值：无
]]
function rsm(s)
	for i=1,#tlist do
		if tlist[i].dat == s then
			tlist[i].cnt = tlist[i].cnt - 1
			if tlist[i].cnt == 0 then
				sys.dispatch("MQTT_DUP_FAIL",tlist[i].typ,tlist[i].seq)
				rmv(nil,s) 
				return 
			end
			tlist[i].tm = DUP_TIME			
			break
		end
	end
end

--[[
函数名：getyp
功能  ：根据序列号查找publish报文用户自定义类型
参数  ：
		seq：publish报文序列号
返回值：无
]]
function getyp(seq)
	for i=1,#tlist do
		if seq and seq == tlist[i].seq then
			return tlist[i].typ
		end
	end
end
