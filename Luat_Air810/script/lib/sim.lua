--[[
模块名称：sim卡功能
模块功能：查询sim卡状态、iccid、imsi、mcc、mnc
模块最后修改时间：2017.02.13
]]

--定义模块,导入依赖库
local string = require"string"
local ril = require"ril"
local sys = require"sys"
local base = _G
local os = require"os"
module(...)

--加载常用的全局函数至本地
local tonumber = base.tonumber
local tostring = base.tostring
local req = ril.request

--sim卡的imsi
local imsi
--sim卡的iccid
local iccid,cpinsta
local smatch = string.match

--[[
函数名：geticcid
功能  ：获取sim卡的iccid
参数  ：无
返回值：iccid，如果还没有读取出来，则返回nil
注意：开机lua脚本运行之后，会发送at命令去查询iccid，所以需要一定时间才能获取到iccid。开机后立即调用此接口，基本上返回nil
]]
function geticcid()
	return iccid or ""
end

--[[
函数名：getimsi
功能  ：获取sim卡的imsi
参数  ：无
返回值：imsi，如果还没有读取出来，则返回nil
注意：开机lua脚本运行之后，会发送at命令去查询imsi，所以需要一定时间才能获取到imsi。开机后立即调用此接口，基本上返回nil
]]
function getimsi()
	return imsi or ""
end

--[[
函数名：getmcc
功能  ：获取sim卡的mcc
参数  ：无
返回值：mcc，如果还没有读取出来，则返回""
注意：开机lua脚本运行之后，会发送at命令去查询imsi，所以需要一定时间才能获取到imsi。开机后立即调用此接口，基本上返回""
]]
function getmcc()
	return (imsi ~= nil and imsi ~= "") and string.sub(imsi,1,3) or ""
end

--[[
函数名：getmnc
功能  ：获取sim卡的getmnc
参数  ：无
返回值：mnc，如果还没有读取出来，则返回""
注意：开机lua脚本运行之后，会发送at命令去查询imsi，所以需要一定时间才能获取到imsi。开机后立即调用此接口，基本上返回""
]]
function getmnc()
	return (imsi ~= nil and imsi ~= "") and string.sub(imsi,4,5) or ""
end

--[[
函数名：getstatus
功能  ：获取sim卡的状态
参数  ：无
返回值：true表示sim卡正常，false或者nil表示未检测到卡或者卡异常
注意：开机lua脚本运行之后，会发送at命令去查询状态，所以需要一定时间才能获取到状态。开机后立即调用此接口，基本上返回nil
]]
function getstatus()
	return cpinsta=="RDY"
end

--[[
函数名：rsp
功能  ：本功能模块内“通过虚拟串口发送到底层core软件的AT命令”的应答处理
参数  ：
		cmd：此应答对应的AT命令
		success：AT命令执行结果，true或者false
		response：AT命令的应答中的执行结果字符串
		intermediate：AT命令的应答中的中间信息
返回值：无
]]
local function rsp(cmd,success,response,intermediate)
	if cmd == "AT+ICCID" then
		iccid = smatch(intermediate,"+ICCID:%s*(%w+)") or ""
	elseif cmd == "AT+CIMI" then
		imsi = intermediate
		--产生一个内部消息IMSI_READY，通知已经读取imsi
		sys.dispatch("IMSI_READY")
	elseif cmd=="AT+CPIN?" then
		--base.print("sim.rsp",cmd,success,response,intermediate)
		if not success or intermediate==nil then
			urc("+CPIN:NOT INSERTED","+CPIN")
		else
			urc(intermediate,smatch(intermediate,"((%+%w+))"))
		end
		ril.regurc("+CPIN",urc)
	end
end

local function setcpinsta(newsta,data)
	if cpinsta~=newsta then
		base.print('setcpinsta',data)
		if newsta=="RDY" then
			req("AT+ICCID")
			req("AT+CIMI")
		end
		cpinsta = newsta
	end
end

--[[
函数名：urc
功能  ：本功能模块内“注册的底层core通过虚拟串口主动上报的通知”的处理
参数  ：
		data：通知的完整字符串信息
		prefix：通知的前缀
返回值：无
]]
function urc(data,prefix)
	--base.print('simurc',data,prefix)
	
	if prefix == "+CPIN" then
		--sim卡正常
		if smatch(data,"+CPIN:%s*READY") then
			setcpinsta("RDY",data)
			sys.dispatch("SIM_IND","RDY")
		--未检测到sim卡
		elseif smatch(data,"+CPIN:%s*NOT INSERTED") then
			setcpinsta("NIST",data)
			sys.dispatch("SIM_IND","NIST")
		else
			setcpinsta("NORDY",data)
			if data == "+CPIN: SIM PIN" then
				sys.dispatch("SIM_IND_SIM_PIN")	
			end
			sys.dispatch("SIM_IND","NORDY")
		end
	elseif prefix == '+ESIMS' then	
		base.print('testetst',data)
		if data == '+ESIMS: 1' then
			setcpinsta("RDY",data)
			sys.dispatch("SIM_IND","RDY")
		else
			setcpinsta("NIST",data)
			sys.dispatch("SIM_IND","NIST")
		end	
	end
end

local function cpinqry()
	ril.regrsp("+CPIN",rsp)
	ril.deregurc("+CPIN")
	req("AT+CPIN?",nil,nil,nil,{skip=true})
end

local function netind(e,v)
	if v=="REGISTERED" then
		cpinqry()
	end	
	return true
end

--注册AT+ICCID命令的应答处理函数
ril.regrsp("+ICCID",rsp)
--注册AT+CIMI命令的应答处理函数
ril.regrsp("+CIMI",rsp)
--注册+CPIN通知的处理函数
ril.regurc("+CPIN",urc)
sys.regapp(netind,"NET_STATE_CHANGED")
sys.timer_loop_start(cpinqry,60000)
