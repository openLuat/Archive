--[[
模块名称：通话测试
模块功能：测试呼入呼出
模块最后修改时间：2017.02.23
]]

module(...,package.seeall)


--[[
函数名：print
功能  ：打印接口，此文件中的所有打印都会加上test前缀
参数  ：无
返回值：无
]]
local function print(...)
	_G.print("test",...)
end

--[[
函数名：connected
功能  ：通话已建立的消息处理函数
参数  ：
		id：通话id
返回值：无
]]
local function connected(id)
	print("connected:"..(id or "nil"))
	--设置mic增益
	audio.setmicrophonegain(7)
	--10秒中之后主动结束通话
	sys.timer_start(cc.hangup,10000,"AUTO_DISCONNECT")
end

--[[
函数名：disconnected
功能  ：通话已结束的消息处理函数
参数  ：
		id：通话id
返回值：无
]]
local function disconnected(id)
	print("disconnected:"..(id or "nil"))
	sys.timer_stop(cc.hangup,"AUTO_DISCONNECT")
end

--表示第几次来电
local incomingIdx = 1
--[[
函数名：incoming
功能  ：来电消息处理函数
参数  ：
		id：通话id
返回值：无
]]
local function incoming(id)
	print("incoming:"..(id or "nil"))
	--第偶数次来电，自动接听
	if incomingIdx%2==0 then
		cc.accept()
	--第奇数次来电，自动挂断
	else
		cc.hangup()
	end	
	incomingIdx = incomingIdx+1
end

local procer =
{
	CALL_INCOMING = incoming, --来电时，lib中的cc.lua会调用sys.dispatch接口抛出CALL_INCOMING消息
	CALL_DISCONNECTED = disconnected,	--通话结束后，lib中的cc.lua会调用sys.dispatch接口抛出CALL_DISCONNECTED消息
}

--下面两行代码是注册消息处理函数的两种方式
--二者的区别是消息处理函数接收到的参数不同
--第一种方式的第一个参数是消息ID
--第二种方式的第一个参数是消息ID后的自定义参数
--请参考incoming，connected，disconnected中的打印
sys.regapp(connected,"CALL_CONNECTED") --建立通话后，lib中的cc.lua会调用sys.dispatch接口抛出CALL_CONNECTED消息
sys.regapp(procer)

--开机后1分钟后呼叫10086
sys.timer_start(cc.dial,60000,"10086")

