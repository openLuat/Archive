--[[
模块名称：通话管理
模块功能：呼入、呼出、接听、挂断
模块最后修改时间：2017.02.20
]]

--定义模块,导入依赖库
local base = _G
local string = require"string"
local table = require"table"
local sys = require"sys"
local ril = require"ril"
local net = require"net"
local pm = require"pm"
local aud = require"audio"
module("cc")

--加载常用的全局函数至本地
local ipairs,pairs = base.ipairs,base.pairs
local dispatch = sys.dispatch
local req = ril.request

--底层通话模块是否准备就绪，true就绪，false或者nil未就绪
local ccready = false
--通话存在标志，在以下状态时为true：
--主叫呼出中，被叫振铃中，通话中
local callexist = false
--记录来电号码保证同一电话多次振铃只提示一次
local incoming_num = nil 
--紧急号码表
local emergency_num = {"112", "911", "000", "08", "110", "119", "118", "999"}
--通话列表
local clcc = {}

--[[
函数名：isemergencynum
功能  ：检查号码是否为紧急号码
参数  ：
		num：待检查号码
返回值：true为紧急号码，false不为紧急号码
]]
function isemergencynum(num)
	for k,v in ipairs(emergency_num) do
		if v == num then
			return true
		end
	end
	return false
end

--[[
函数名：clearincomingflag
功能  ：清除来电号码
参数  ：无
返回值：无
]]
local function clearincomingflag()
	incoming_num = nil
end

--[[
函数名：discevt
功能  ：通话结束消息处理
参数  ：
		reason：结束原因
返回值：无
]]
local function discevt(reason)
	callexist = false -- 通话结束 清除通话状态标志
	if incoming_num then sys.timer_start(clearincomingflag,1000) end
	pm.sleep("cc")
	--产生内部消息CALL_DISCONNECTED，通知用户程序通话结束
	dispatch("CALL_DISCONNECTED",reason)
end

--[[
函数名：anycallexist
功能  ：是否存在通话
参数  ：无
返回值：存在通话返回true，否则返回false
]]
function anycallexist()
	return callexist
end

--[[
函数名：qrylist
功能  ：查询通话列表
参数  ：无
返回值：无
]]
local function qrylist()
	clcc = {}
	req("AT+CLCC")
end

local function proclist()
	local k,v,isactive
	for k,v in pairs(clcc) do
		if v.sta == "0" then isactive = true break end
	end
	if isactive and #clcc > 1 then
		for k,v in pairs(clcc) do
			if v.sta ~= "0" then req("AT+CHLD=1"..v.id) end
		end
	end
end

--[[
函数名：dial
功能  ：呼叫一个号码
参数  ：
		number：号码
		delay：延时delay毫秒后，才发送at命令呼叫，默认不延时
返回值：无
]]
function dial(number,delay)
	if number == "" or number == nil then
		return false
	end

	if ccready == false and not isemergencynum(number) then
		return false
	end

	pm.wake("cc")
	req(string.format("%s%s;","ATD",number),nil,nil,delay)
	callexist = true -- 主叫呼出

	return true
end

--[[
函数名：hangup
功能  ：主动挂断所有通话
参数  ：无
返回值：无
]]
function hangup()
	aud.stop()
	req("AT+CHUP")
end

--[[
函数名：accept
功能  ：接听来电
参数  ：无
返回值：无
]]
function accept()
	aud.stop()
	req("ATA")
	pm.wake("cc")
end

--[[
函数名：ccurc
功能  ：本功能模块内“注册的底层core通过虚拟串口主动上报的通知”的处理
参数  ：
		data：通知的完整字符串信息
		prefix：通知的前缀
返回值：无
]]
local function ccurc(data,prefix)
	--底层通话模块准备就绪
	if data == "CALL READY" then
		ccready = true
		dispatch("CALL_READY")
		req("AT+CCWA=1")
	--通话建立通知
	elseif data == "CONNECT" then
		qrylist()
		dispatch("CALL_CONNECTED")
	--通话挂断通知
	elseif data == "NO CARRIER" or data == "BUSY" or data == "NO ANSWER" then
		qrylist()
		discevt(data)
	--来电振铃
	elseif prefix == "+CLIP" then
		qrylist()
		local number = string.match(data,"\"(%+*%d*)\"",string.len(prefix)+1)
		callexist = true -- 被叫振铃
		if incoming_num ~= number then
			incoming_num = number
			dispatch("CALL_INCOMING",number)
		end
	elseif prefix == "+CCWA" then
		qrylist()
	--通话列表信息
	elseif prefix == "+CLCC" then
		local id,dir,sta = string.match(data,"%+CLCC:%s*(%d+),(%d),(%d)")
		if id then
			table.insert(clcc,{id=id,dir=dir,sta=sta})
			proclist()
		end
	end
end

--[[
函数名：ccrsp
功能  ：本功能模块内“通过虚拟串口发送到底层core软件的AT命令”的应答处理
参数  ：
		cmd：此应答对应的AT命令
		success：AT命令执行结果，true或者false
		response：AT命令的应答中的执行结果字符串
		intermediate：AT命令的应答中的中间信息
返回值：无
]]
local function ccrsp(cmd,success,response,intermediate)
	local prefix = string.match(cmd,"AT(%+*%u+)")
	--拨号应答
	if prefix == "D" then
		if not success then
			discevt("CALL_FAILED")
		end
	--挂断所有通话应答
	elseif prefix == "+CHUP" then
		discevt("LOCAL_HANG_UP")
	--接听来电应答
	elseif prefix == "A" then
		incoming_num = nil
		dispatch("CALL_CONNECTED")
	end
	qrylist()
end

--注册以下通知的处理函数
ril.regurc("CALL READY",ccurc)
ril.regurc("CONNECT",ccurc)
ril.regurc("NO CARRIER",ccurc)
ril.regurc("NO ANSWER",ccurc)
ril.regurc("BUSY",ccurc)
ril.regurc("+CLIP",ccurc)
ril.regurc("+CLCC",ccurc)
ril.regurc("+CCWA",ccurc)
--注册以下AT命令的应答处理函数
ril.regrsp("D",ccrsp)
ril.regrsp("A",ccrsp)
ril.regrsp("+CHUP",ccrsp)
ril.regrsp("+CHLD",ccrsp)

--开启拨号音,忙音检测
req("ATX4") 
--开启来电urc上报
req("AT+CLIP=1")
