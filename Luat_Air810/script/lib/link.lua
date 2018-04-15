--[[
模块名称：数据链路、SOCKET管理
模块功能：数据网络激活，SOCKET的创建、连接、数据收发、状态维护
模块最后修改时间：2017.02.14
]]

--定义模块,导入依赖库
local base = _G
local string = require"string"
local table = require"table"
local sys = require"sys"
local ril = require"ril"
local net = require"net"
local rtos = require"rtos"
local sim = require"sim"
local socket = require"tcpipsock"
module(...,package.seeall)

--加载常用的全局函数至本地
local print = base.print
local pairs = base.pairs
local tonumber = base.tonumber
local tostring = base.tostring
local req = ril.request
local extapn
--最大socket id，从0开始，所以同时支持的socket连接数是8个
local MAXLINKS = 7
--IP环境建立失败时间隔5秒重连
local IPSTART_INTVL = 5000

--socket连接表
local linklist = {}
--ipstatus：IP环境状态
--sckconning：是否连接数据网络
--shuting：是否正在关闭数据网络
local ipstatus,sckconning,shuting = "IP INITIAL"
--GPRS数据网络附着状态，"1"附着，其余未附着
local cgatt
--apn，用户名，密码
local apnname = "CMNET"
local username=''
local password=''
--socket发起连接请求后，如果在connectnoretinterval毫秒后没有任何应答，如果connectnoretrestart为true，则会重启软件
local connectnoretrestart = false
local connectnoretinterval
--apnflg：本功能模块是否自动获取apn信息，true是，false则由用户应用脚本自己调用setapn接口设置apn、用户名和密码
--checkciicrtm：执行AT+CIICR后，如果设置了checkciicrtm，checkciicrtm毫秒后，没有激活成功，则重启软件（中途执行AT+CIPSHUT则不再重启）
--flymode：是否处于飞行模式
--updating：是否正在执行远程升级功能(update.lua)
--dbging：是否正在执行dbg功能(dbg.lua)
--ntping：是否正在执行NTP时间同步功能(ntp.lua)
--shutpending：是否有等待处理的进入AT+CIPSHUT请求
local apnflag,checkciicrtm,flymode,updating,dbging,ntping,shutpending,pdpdeacting=true


--[[
函数名：print
功能  ：打印接口，此文件中的所有打印都会加上link前缀
参数  ：无
返回值：无
]]
local function print(...)
	base.print("link",...)
end

--[[
函数名：setapn
功能  ：设置apn、用户名和密码
参数  ：
		a：apn
		b：用户名
		c：密码
返回值：无
]]
function setapn(a,b,c)
	apnname,username,password = a,b or '',c or ''
	apnflag=false
end

--[[
函数名：getapn
功能  ：获取apn
参数  ：无
返回值：apn
]]
function getapn()
	return apnname
end

--[[
函数名：connectingtimerfunc
功能  ：socket连接超时没有应答处理函数
参数  ：
		id：socket id
返回值：无
]]
local function connectingtimerfunc(id)
	print("connectingtimerfunc",id,connectnoretrestart)
	if connectnoretrestart then
		sys.restart("link.connectingtimerfunc")
	end
end

--[[
函数名：stopconnectingtimer
功能  ：关闭“socket连接超时没有应答”定时器
参数  ：
		id：socket id
返回值：无
]]
local function stopconnectingtimer(id)
	print("stopconnectingtimer",id)
	sys.timer_stop(connectingtimerfunc,id)
end

--[[
函数名：startconnectingtimer
功能  ：开启“socket连接超时没有应答”定时器
参数  ：
		id：socket id
返回值：无
]]
local function startconnectingtimer(id)
	print("startconnectingtimer",id,connectnoretrestart,connectnoretinterval)
	if id and connectnoretrestart and connectnoretinterval and connectnoretinterval > 0 then
		sys.timer_start(connectingtimerfunc,connectnoretinterval,id)
	end
end

--[[
函数名：setconnectnoretrestart
功能  ：设置“socket连接超时没有应答”的控制参数
参数  ：
		flag：功能开关，true或者false
		interval：超时时间，单位毫秒
返回值：无
]]
function setconnectnoretrestart(flag,interval)
	connectnoretrestart = flag
	connectnoretinterval = interval
end

--[[
函数名：setupIP
功能  ：发送激活IP网络请求
参数  ：无
返回值：无
]]
function setupIP()
	print("setupIP:",ipstatus,cgatt,flymode)
	--数据网络已激活或者处于飞行模式，直接返回
	if ipstatus ~= "IP INITIAL" or flymode then
		return
	end
	--gprs数据网络没有附着上
	if cgatt ~= "1" then
		print("setupIP: wait cgatt")
		return
	end

	socket.pdp_activate(apnname,username,password)
end

--[[
函数名：emptylink
功能  ：获取可用的socket id
参数  ：无
返回值：可用的socket id，如果没有可用的返回nil
]]
local function emptylink()
	for i = 0,MAXLINKS do
		if linklist[i] == nil then
			return i
		end
	end

	return nil
end

--[[
函数名：validaction
功能  ：检查某个socket id的动作是否有效
参数  ：
		id：socket id
		action：动作
返回值：true有效，false无效
]]
local function validaction(id,action)
	--socket无效
	if linklist[id] == nil then
		print("validaction:id nil",id)
		return false
	end

	--同一个状态不重复执行
	if action.."ING" == linklist[id].state then
		print("validaction:",action,linklist[id].state)
		return false
	end

	local ing = string.match(linklist[id].state,"(ING)",-3)

	if ing then
		--有其他任务在处理时,不允许处理连接,断链或者关闭是可以的
		if action == "CONNECT" and linklist[id].state~= "SHUTING" then
			print("validaction: action running",linklist[id].state,action)
			return false
		end
	end

	-- 无其他任务在执行,允许执行
	return true
end

--[[
函数名：openid
功能  ：保存socket的参数信息
参数  ：
		id：socket id
		notify：socket状态处理函数
		recv：socket数据接收处理函数
		tag：socket创建标记
返回值：true成功，false失败
]]
function openid(id,notify,recv,tag)
	--id越界或者id的socket已经存在
	if id > MAXLINKS or linklist[id] ~= nil then
		print("error",id)
		return false
	end

	local link = {
		notify = notify,
		recv = recv,
		state = "INITIAL",
		tag = tag,
	}

	linklist[id] = link

	--激活IP网络
	if ipstatus ~= "IP STATUS" and ipstatus ~= "IP PROCESSING" then
		setupIP()
	end

	return true
end

--[[
函数名：open
功能  ：创建一个socket
参数  ：
		notify：socket状态处理函数
		recv：socket数据接收处理函数
		tag：socket创建标记
返回值：number类型的id表示成功，nil表示失败
]]
function open(notify,recv,tag)
	local id = emptylink()

	if id == nil then
		return nil,"no empty link"
	end

	openid(id,notify,recv,tag)

	return id
end

--[[
函数名：close
功能  ：关闭一个socket（会清除socket的所有参数信息）
参数  ：
		id：socket id
返回值：true成功，false失败
]]
function close(id)
	print("close",id)
	--检查是否允许关闭
	if validaction(id,"CLOSE") == false then
		return false
	end
	--正在关闭
	linklist[id].state = "CLOSING"

	socket.sock_close(id,1)

	return true
end

--[[
函数名：asyncLocalEvent
功能  ：socket异步通知消息的处理函数
参数  ：
		msg：异步通知消息"LINK_ASYNC_LOCAL_EVENT"
		cbfunc：消息回调
		id：socket id
		val：通知消息的参数
返回值：true成功，false失败
]]
function asyncLocalEvent(msg,cbfunc,id,val)
	cbfunc(id,val)
end

--注册消息LINK_ASYNC_LOCAL_EVENT的处理函数
sys.regapp(asyncLocalEvent,"LINK_ASYNC_LOCAL_EVENT")

--[[
函数名：connect
功能  ：socket连接服务器请求
参数  ：
		id：socket id
		protocol：传输层协议，TCP或者UDP
		address：服务器地址
		port：服务器端口
返回值：请求成功同步返回true，否则false；
]]
function connect(id,protocol,address,port)
	print("connect",id,protocol,address,port,ipstatus,sckconning,shuting)
	--不允许发起连接动作
	if validaction(id,"CONNECT") == false or linklist[id].state == "CONNECTED" then
		return false
	end

	if cc and cc.anycallexist() then
		--如果打开了通话功能 并且当前正在通话中使用异步通知连接失败
		print("connect:failed cause call exist")
		sys.dispatch("LINK_ASYNC_LOCAL_EVENT",statusind,id,"CONNECT FAIL")
		return true
	end

	local connstr = string.format("AT+CIPSTART=%d,\"%s\",\"%s\",%s",id,protocol,address,port)
	linklist[id].state = "CONNECTING"
	if (ipstatus ~= "IP STATUS" and ipstatus ~= "IP PROCESSING") or sckconning or shuting then
		-- ip环境未准备好先加入等待
		linklist[id].pending = connstr
	else
		socket.sock_conn(id,(protocol=="TCP") and 0 or 1,tonumber(port),address)
		startconnectingtimer(id)
		sckconning = true
	end

	return true
end

--[[
函数名：disconnect
功能  ：断开一个socket（不会清除socket的所有参数信息）
参数  ：
		id：socket id
返回值：true成功，false失败
]]
function disconnect(id)
	print("disconnect",id)
	--不允许断开动作
	if validaction(id,"DISCONNECT") == false then
		return false
	end
	--如果此socket id对应的连接还在等待中，并没有真正发起
	if linklist[id].pending then
		linklist[id].pending = nil
		if ipstatus ~= "IP STATUS" and ipstatus ~= "IP PROCESSING" and linklist[id].state == "CONNECTING" then
			print("disconnect: ip not ready",ipstatus)
			linklist[id].state = "DISCONNECTING"
			sys.dispatch("LINK_ASYNC_LOCAL_EVENT",closecnf,id,"DISCONNECT","OK")
			return
		end
	end

	linklist[id].state = "DISCONNECTING"
	--断开连接
	socket.sock_close(id,1)

	return true
end

--[[
函数名：send
功能  ：发送数据到服务器
参数  ：
		id：socket id
		data：要发送的数据
返回值：true成功，false失败
]]
function send(id,data)
	--socket无效，或者socket未连接
	if linklist[id] == nil or linklist[id].state ~= "CONNECTED" then
		print("send:error",id)
		return false
	end

	if cc and cc.anycallexist() then
		-- 如果打开了通话功能 并且当前正在通话中使用异步通知连接失败
		print("send:failed cause call exist")
		return false
	end
	--执行数据发送
	print("send",id,string.len(data),(string.len(data) > 200) and "" or data)
	socket.sock_send(id,data)

	return true
end

--[[
函数名：getstate
功能  ：获取一个socket的连接状态
参数  ：
		id：socket id
返回值：socket有效则返回连接状态，否则返回"NIL LINK"
]]
function getstate(id)
	return linklist[id] and linklist[id].state or "NIL LINK"
end

--[[
函数名：recv
功能  ：某个socket的数据接收处理函数
参数  ：
		id：socket id
		data：接收到的数据内容
返回值：无
]]
local function recv(id,data)
	print("recv",id,string.len(data)>200 and "" or data)
	if not id or not linklist[id] then
		print("recv:error",id)
		return
	end
	--调用socket id对应的用户注册的数据接收处理函数
	if linklist[id].recv then
		linklist[id].recv(id,data)
	else
		print("recv:nil recv",id)
	end
end

--[[ ipstatus查询返回的状态不提示
function linkstatus(data)
end
]]

--[[
函数名：usersckisactive
功能  ：判断用户创建的socket连接是否处于激活状态
参数  ：无
返回值：只要任何一个用户socket处于连接状态就返回true，否则返回nil
]]
local function usersckisactive()
	for i = 0,MAXLINKS do
		--用户自定义的socket，没有tag值
		if linklist[i] and not linklist[i].tag and linklist[i].state=="CONNECTED" then
			return true
		end
	end
end

--[[
函数名：usersckntfy
功能  ：用户创建的socket连接状态变化通知
参数  ：
		id：socket id
返回值：无
]]
local function usersckntfy(id)
	--产生一个内部消息"USER_SOCKET_CONNECT"，通知“用户创建的socket连接状态发生变化”
	if not linklist[id].tag then sys.dispatch("USER_SOCKET_CONNECT",usersckisactive()) end
end

--[[
函数名：sendcnf
功能  ：socket数据发送结果确认
参数  ：
		id：socket id
		result：发送结果字符串
返回值：无
]]
local function sendcnf(id,result)
	if not id or not linklist[id] then print("sendcnf:error",id) return end
	local str = string.match(result,"([%u ])")
	--发送失败
	if str == "TCP ERROR" or str == "UDP ERROR" or str == "ERROR" then
		linklist[id].state = result
	end
	--调用用户注册的状态处理函数
	linklist[id].notify(id,"SEND",result)
end

--[[
函数名：closecnf
功能  ：socket关闭结果确认
参数  ：
		id：socket id
		result：关闭结果字符串
返回值：无
]]
function closecnf(id,result)	
	--socket id无效
	if not id or not linklist[id] then
		print("closecnf:error",id)
		return
	end
	print("closecnf",id,result,linklist[id].state,shuting)
	--不管任何的close结果,链接总是成功断开了,所以直接按照链接断开处理
	if linklist[id].state == "DISCONNECTING" then
		linklist[id].state = "CLOSED"
		linklist[id].notify(id,"DISCONNECT","OK")
		usersckntfy(id,false)
		stopconnectingtimer(id)
	--连接注销,清除维护的连接信息,清除urc关注
	elseif linklist[id].state == "CLOSING" then		
		local tlink = linklist[id]
		usersckntfy(id,false)
		linklist[id] = nil
		tlink.notify(id,"CLOSE","OK")		
		stopconnectingtimer(id)
	elseif linklist[id].state == "SHUTING" or shuting then
		linklist[id].state = "CLOSED"
		linklist[id].notify(id,"STATE","SHUTED")
	else
		print("closecnf:error",linklist[id].state)
	end
end

--[[
函数名：statusind
功能  ：socket状态转化处理
参数  ：
		id：socket id
		state：状态字符串
返回值：无
]]
function statusind(id,state)	
	--socket无效
	if linklist[id] == nil then
		print("statusind:nil id",id)
		return
	end
	
	print("statusind",id,state,linklist[id].state)

    -- 快发失败的提示会变成URC
	if state == "SEND FAIL" then
		if linklist[id].state == "CONNECTED" then
			linklist[id].notify(id,"SEND",state)
		else
			print("statusind:send fail state",linklist[id].state)
		end
		return
	end

	local evt
	--socket如果处于正在连接的状态，或者返回了连接成功的状态通知
	if linklist[id].state == "CONNECTING" or state == "CONNECT OK" then
		--连接类型的事件
		evt = "CONNECT"		
	else
		--状态类型的事件
		evt = "STATE"
	end

	--除非连接成功,否则连接仍然还是在关闭状态
	if state == "CONNECT OK" then
		linklist[id].state = "CONNECTED"		
	else
		linklist[id].state = "CLOSED"
	end
	--调用usersckntfy判断是否需要通知“用户socket连接状态发生变化”
	usersckntfy(id,state == "CONNECT OK")
	--调用用户注册的状态处理函数
	linklist[id].notify(id,evt,state)
	stopconnectingtimer(id)
end

--[[
函数名：connpend
功能  ：执行因IP网络未准备好被挂起的socket连接请求
参数  ：无
返回值：无
]]
local function connpend()
	for k,v in pairs(linklist) do
		print("connpend",v.pending)
		if v.pending then
			local id,protocol,address,port = string.match(v.pending,"AT%+CIPSTART=(%d+),\"(%a+)\",\"(.+)\",(%d+)")
			if id then
				id = tonumber(id)
				socket.sock_conn(id,(protocol=="TCP") and 0 or 1,tonumber(port),address)
				linklist[id].state = "CONNECTING"
				startconnectingtimer(id)
				sckconning = true
			end
			v.pending = nil
			break
		end
	end	
end

local ipstatusind
function regipstatusind()
	ipstatusind = true
end

--[[
函数名：setIPStatus
功能  ：设置IP网络状态
参数  ：
		status：IP网络状态
返回值：无
]]
local function setIPStatus(status)
	print("setIPStatus:",status,ipstatus)

	if ipstatus ~= status then
		if ipstatusind then
			sys.dispatch("IP_STATUS_IND",status=="IP GPRSACT" or status=="IP PROCESSING" or status=="IP STATUS")
		end

		ipstatus = status
		if ipstatus == "IP STATUS" then
			connpend()
		elseif ipstatus == "IP INITIAL" then -- 重新连接
			sys.timer_start(setupIP,IPSTART_INTVL)		
		else -- 其他异常状态关闭至IP INITIAL
			shut()
		end
	elseif status == "IP INITIAL" then
		sys.timer_start(setupIP,IPSTART_INTVL)
	end
end

--[[
函数名：closeall
功能  ：关闭全部IP网络
参数  ：无
返回值：无
]]
local function closeall()
	shuting = false
	if ipstatusind then sys.dispatch("IP_SHUTING_IND",false) end
	local i

	for i = 0,MAXLINKS do
		if linklist[i] then
			print("closeall",linklist[i].state,linklist[i].pending)
			if linklist[i].state == "CONNECTING" and linklist[i].pending then
				-- 对于尚未进行过的连接请求 不提示close,IP环境建立后自动连接
			elseif linklist[i].state == "INITIAL" then -- 未连接的也不提示
			else
				linklist[i].state = "SHUTING"
				usersckntfy(i,false)
				socket.sock_close(i,0)
			end
			stopconnectingtimer(i)
		end
	end
end

local function pdpdeact()
	socket.pdp_deactivate()
	pdpdeacting = true
	sys.timer_start(pdpdeactrsp,10000,true)
end

--[[
函数名：shut
功能  ：关闭IP网络
参数  ：无
返回值：无
]]
function shut()
	print("shut",updating,dbging,ntping,shutpending)
	--如果正在执行远程升级功能或者dbg功能或者ntp功能，则延迟关闭
	if updating or dbging or ntping then shutpending = true return end
	closeall()
	pdpdeact()
	sckconning = false
	--设置关闭中标志
	shuting = true
	if ipstatusind then sys.dispatch("IP_SHUTING_IND",true) end
	shutpending = false
end
reset = shut

function pdpdeactrsp(tmout)
	print("pdpdeactrsp",tmout,ipstatus)
	pdpdeacting = false
	if tmout and ipstatus == "IP STATUS" then
		connpend()
	end
	sys.timer_stop(pdpdeactrsp,true)
end

local function pdpactcnf(result)
	if result == 1 then
		pdpdeactrsp()
		setIPStatus("IP STATUS")
	else
		pdpdeact()
	end
end

local function pdpdeactcnf(result)
	shuting = false
	pdpdeactrsp()
	setIPStatus("IP INITIAL")
end

local function pdpdeactind(result)
	closeall()
	setIPStatus("IP INITIAL")
end

local function sockconncnf(id,result)
	statusind(id,(result == 1) and "CONNECT OK" or "ERROR")
	sckconning = nil
	connpend()
end

local function socksendcnf(id,result)
	sendcnf(id,(result == 1) and "SEND OK" or "ERROR")
end

local function sockrecvind(id,cnt)
	if cnt > 0 then
		recv(id,socket.sock_recv(id,cnt))
	end
end

local function sockclosecnf(id,result)
	closecnf(id,(result == 1) and "CLOSE OK" or "ERROR")
end

local function sockcloseind(id,result)
	statusind(id,"CLOSED")
end

local tntfy =
{
	[rtos.MSG_PDP_ACT_CNF] = {nm="PDP_ACT_CNF",hd=pdpactcnf},
	[rtos.MSG_PDP_DEACT_CNF] = {nm="PDP_DEACT_CNF",hd=pdpdeactcnf},
	[rtos.MSG_PDP_DEACT_IND] = {nm="PDP_DEACT_IND",hd=pdpdeactind},
	[rtos.MSG_SOCK_CONN_CNF] = {nm="SOCK_CONN_CNF",hd=sockconncnf},
	[rtos.MSG_SOCK_SEND_CNF] = {nm="SOCK_SEND_CNF",hd=socksendcnf},
	[rtos.MSG_SOCK_RECV_IND] = {nm="SOCK_RECV_IND",hd=sockrecvind},
	[rtos.MSG_SOCK_CLOSE_CNF] = {nm="SOCK_CLOSE_CNF",hd=sockclosecnf},
	[rtos.MSG_SOCK_CLOSE_IND] = {nm="SOCK_CLOSE_IND",hd=sockcloseind},
}

local function ntfy(msg,v1,v2,v3)
	print("ntfy",tntfy[msg] and tntfy[msg].nm or "unknown",v1,v2,v3)
	if tntfy[msg] then		
		tntfy[msg].hd(v1,v2,v3)
	end
end

sys.regmsg("sock",ntfy)

--gprs网络未附着时，定时查询附着状态的间隔
local QUERYTIME = 2000
local querycgatt

--[[
函数名：cgattrsp
功能  ：查询GPRS数据网络附着状态的应答处理
参数  ：
		cmd：此应答对应的AT命令
		success：AT命令执行结果，true或者false
		response：AT命令的应答中的执行结果字符串
		intermediate：AT命令的应答中的中间信息
返回值：无
]]
local function cgattrsp(cmd,success,response,intermediate)
	--已附着
	if intermediate == "+CGATT: 1" then
		if cgatt ~= "1" then
			cgatt = "1"
			sys.dispatch("NET_GPRS_READY",true)

			-- 如果存在链接,那么在gprs附着上以后自动初始化IP环境
			if base.next(linklist) then
				if ipstatus == "IP INITIAL" then
					setupIP()
				else
					setIPStatus("IP STATUS")
				end
			end
		end
		sys.timer_start(querycgatt,60000)
	elseif intermediate == "+CGATT: 0" then
		if cgatt ~= "0" then
			cgatt = "0"
			sys.dispatch("NET_GPRS_READY",false)
		end
		setcgatt(1)
		sys.timer_start(querycgatt,QUERYTIME)
	end
end

--[[
函数名：querycgatt
功能  ：查询GPRS数据网络附着状态
参数  ：无
返回值：无
]]
function querycgatt()
	--不是飞行模式，才去查询
	if not flymode then req("AT+CGATT?",nil,cgattrsp,nil,{skip=true}) end
end

function setquicksend() end

--[[
函数名：netmsg
功能  ：GSM网络注册状态发生变化的处理
参数  ：无
返回值：true
]]
local function netmsg(id,data)
	print("netmsg",data)
	if data == "REGISTERED" then
		--mtk需要主动attach
		setcgatt(1)
		sys.timer_start(querycgatt,QUERYTIME)
	elseif  data == "UNREGISTER" then 
		if cgatt ~= "0" then
			cgatt = "0"
			sys.dispatch("NET_GPRS_READY",false)
		end	
	end
	return true
end

--sim卡的默认apn表
local apntable =
{
	["46000"] = "CMNET",
	["46002"] = "CMNET",
	["46004"] = "CMNET",
	["46007"] = "CMNET",
	["46001"] = "UNINET",
	["46006"] = "UNINET",
	["46009"] = "3GNET",
}

--[[
函数名：proc
功能  ：本模块注册的内部消息的处理函数
参数  ：
		id：内部消息id
		para：内部消息参数
返回值：true
]]
local function proc(id,para)
	--IMSI读取成功
	if id=="IMSI_READY" then
	if not extapn then
		apnname=apntable[sim.getmcc()..sim.getmnc()] or "CMNET"
	end
	--飞行模式状态变化
	elseif id=="FLYMODE_IND" then
		flymode = para
		if para then
			sys.timer_stop(req,"AT+CIPSTATUS")
		else
			req("AT+CGATT?",nil,cgattrsp)
		end
	--远程升级开始
	elseif id=="UPDATE_BEGIN_IND" then
		updating = true
	--远程升级结束
	elseif id=="UPDATE_END_IND" then
		updating = false
		if shutpending then shut() end
	--dbg功能开始
	elseif id=="DBG_BEGIN_IND" then
		dbging = true
	--dbg功能结束
	elseif id=="DBG_END_IND" then
		dbging = false
		if shutpending then shut() end
	--NTP同步开始
	elseif id=="NTP_BEGIN_IND" then
		ntping = true
	--NTP同步结束
	elseif id=="NTP_END_IND" then
		ntping = false
		if shutpending then shut() end
	end
	return true
end

function setcgatt(v)
	req("AT+CGATT="..v,nil,nil,nil,{skip=true})
end

function getcgatt()
	return cgatt
end

function getipstatus()
	return ipstatus
end

--注册本模块关注的内部消息的处理函数
sys.regapp(proc,"IMSI_READY","FLYMODE_IND","UPDATE_BEGIN_IND","UPDATE_END_IND","DBG_BEGIN_IND","DBG_END_IND","NTP_BEGIN_IND","NTP_END_IND")
sys.regapp(netmsg,"NET_STATE_CHANGED")

