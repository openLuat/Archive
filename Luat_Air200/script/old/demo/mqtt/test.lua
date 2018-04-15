module(...,package.seeall)

--[[
测试时请搭建自己的服务器，并且修改下面的PROT，ADDR，PORT 
]]

local ssub,schar,smatch,sbyte,slen = string.sub,string.char,string.match,string.byte,string.len
--测试时请搭建自己的服务器
local SCK_IDX,PROT,ADDR,PORT = 1,"TCP","www.test.com",1884
--linksta:与后台的socket连接状态
local linksta
--一个连接周期内的动作：如果连接后台失败，会尝试重连，重连间隔为RECONN_PERIOD秒，最多重连RECONN_MAX_CNT次
--如果一个连接周期内都没有连接成功，则等待RECONN_CYCLE_PERIOD秒后，重新发起一个连接周期
--如果连续RECONN_CYCLE_MAX_CNT次的连接周期都没有连接成功，则重启软件
local RECONN_MAX_CNT,RECONN_PERIOD,RECONN_CYCLE_MAX_CNT,RECONN_CYCLE_PERIOD = 3,5,3,20
--reconncnt:当前连接周期内，已经重连的次数
--reconncyclecnt:连续多少个连接周期，都没有连接成功
--一旦连接成功，都会复位这两个标记
--reconning:是否在尝试连接
local reconncnt,reconncyclecnt,reconning = 0,0
--KEEP_ALIVE_TIME：mqtt保活时间
--rcvs：从后台收到的数据
local KEEP_ALIVE_TIME,rcvs = 600,""

--[[
目前只支持QoS=0和QoS=1，不支持QoS=2
topic、client identifier、user、password只支持ascii字符串

功能如下：
1、终端订阅了"/v1/device/"..misc.getimei().."/devparareq/+"和"/v1/device/"..misc.getimei().."/deveventreq/+"两个主题，参考函数mqttsubdata
2、连接上后台后，终端每隔1分钟分别会发送一个qos为0和1的PUBLISH报文，参考loc0snd和loc1snd
]]

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
函数名：enpwd
功能  ：MQTT CONNECT报文中password字段用到的加密算法
参数  ：
		s：ascii字符串
返回值：加密后的ascii字符串
]]
local function enpwd(s)
	local tmp,ret,i = 0,""
	for i=1,string.len(s) do
		tmp = bit.bxor(tmp,string.byte(s,i))
		if i % 3 == 0 then
			ret = ret..schar(tmp)
			tmp = 0
		end
	end
	return common.binstohexs(ret)
end

--[[
函数名：mqttconncb
功能  ：发送MQTT CONNECT报文后的异步回调函数
参数  ：		
		result： bool类型，发送结果，true为成功，其他为失败
		data：MQTT CONNECT报文数据
返回值：无
]]
function mqttconncb(result,data)
	--把MQTT CONNECT报文数据保存起来，如果超时DUP_TIME秒中没有收到CONNACK或者CONNACK返回失败，则会自动重发CONNECT报文
	--重发的触发开关在mqttdup.lua中
	mqttdup.ins(tmqttpack["MQTTCONN"].mqttduptyp,data)
end

--[[
函数名：mqttconndata
功能  ：组包MQTT CONNECT报文数据
参数  ：无		
返回值：CONNECT报文数据和报文参数
]]
function mqttconndata()
	return mqtt.pack(mqtt.CONNECT,KEEP_ALIVE_TIME,misc.getimei(),misc.getimei(),enpwd(misc.getimei()))
end

--[[
函数名：mqttsubcb
功能  ：发送MQTT SUBSCRIBE报文后的异步回调函数
参数  ：		
		result： bool类型，发送结果，true为成功，其他为失败
		v：SUBSCRIBE报文的参数，table类型{dup=true,topic=mqttsubdata中组包时的topic,seq=mqttsubdata中组包时生成的序列号}
返回值：无
]]
function mqttsubcb(result,v)
	--重新封装MQTT SUBSCRIBE报文，重复标志设为true，序列号和topic都是用原始值，数据保存起来，如果超时DUP_TIME秒中没有收到SUBACK，则会自动重发SUBSCRIBE报文
	--重发的触发开关在mqttdup.lua中
	mqttdup.ins(tmqttpack["MQTTSUB"].mqttduptyp,mqtt.pack(mqtt.SUBSCRIBE,v),v.seq)
end

--[[
函数名：mqttsubdata
功能  ：组包MQTT SUBSCRIBE报文数据
参数  ：无		
返回值：SUBSCRIBE报文数据和报文参数
]]
function mqttsubdata()
	return mqtt.pack(mqtt.SUBSCRIBE,{topic={"/v1/device/"..misc.getimei().."/devparareq/+", "/v1/device/"..misc.getimei().."/deveventreq/+"}})
end

--[[
函数名：mqttdiscb
功能  ：发送MQTT DICONNECT报文后的异步回调函数
参数  ：		
		result： bool类型，发送结果，true为成功，其他为失败
		v：DICONNECT报文的参数，目前固定为"MQTTDISC"，无意义
返回值：无
]]
function mqttdiscb(result,v)
	--关闭socket连接
	linkapp.sckdisc(SCK_IDX)
end

--[[
函数名：mqttdiscdata
功能  ：组包MQTT DISCONNECT报文数据
参数  ：无		
返回值：DISCONNECT报文数据和报文参数
]]
function mqttdiscdata()
	return mqtt.pack(mqtt.DISCONNECT)
end

--[[
函数名：disconnect
功能  ：发送MQTT DISCONNECT报文
参数  ：无		
返回值：无
]]
local function disconnect()
	mqttsnd("MQTTDISC")
end

--[[
函数名：mqttpingreqdata
功能  ：组包MQTT PINGREQ报文数据
参数  ：无		
返回值：PINGREQ报文数据和报文参数
]]
function mqttpingreqdata()
	return mqtt.pack(mqtt.PINGREQ)
end

--[[
函数名：pingreq
功能  ：发送MQTT PINGREQ报文
参数  ：无		
返回值：无
]]
local function pingreq()
	mqttsnd("MQTTPINGREQ")
	if not sys.timer_is_active(disconnect) then
		--启动定时器：如果保活时间+30秒内，没有收到pingrsp，则发送MQTT DISCONNECT报文
		sys.timer_start(disconnect,(KEEP_ALIVE_TIME+30)*1000)
	end
end

--[[
函数名：mqttpubloc0cb
功能  ：发送qos为0的MQTT PUBLISH报文后的异步回调函数
参数  ：		
		result： bool类型，发送结果，true为成功，其他为失败
		v：PUBLISH报文的参数，目前固定为"MQTTPUBLOC0"，无意义
返回值：无
]]
function mqttpubloc0cb(result,v)
	--启动定时器，60秒后再次发送qos为0的PULISH报文
	sys.timer_start(loc0snd,60000)
end

--[[
函数名：mqttpubloc0data
功能  ：组包qos为0的MQTT PUBLISH报文数据
参数  ：无		
返回值：PUBLISH报文数据和报文参数
]]
function mqttpubloc0data()
	return mqtt.pack(mqtt.PUBLISH,{qos=0,topic="/v1/device/"..misc.getimei().."/devdata",payload="loc data0"})
end

--[[
函数名：loc0snd
功能  ：发送qos为0的MQTT PUBLISH报文
参数  ：无		
返回值：无
]]
function loc0snd()
	mqttsnd("MQTTPUBLOC0")
end


--[[
函数名：mqttpubloc1cb
功能  ：发送qos为1的MQTT PUBLISH报文后的异步回调函数
参数  ：		
		result： bool类型，发送结果，true为成功，其他为失败
		v：PUBLISH报文的参数，table类型{dup=true,topic=mqttpubloc1data中组包时的topic,seq=mqttpubloc1data中组包时生成的序列号,payload=mqttpubloc1data中组包时的payload}
返回值：无
]]
function mqttpubloc1cb(result,v)
	--启动定时器，60秒后再次发送qos为1的PULISH报文
	sys.timer_start(loc1snd,60000)
	--重新封装MQTT PUBLISH报文，重复标志设为true，序列号、topic、payload都是用原始值，数据保存起来，如果超时DUP_TIME秒中没有收到PUBACK，则会自动重发PUBLISH报文
	--重发的触发开关在mqttdup.lua中
	mqttdup.ins(tmqttpack["MQTTPUBLOC1"].mqttduptyp,mqtt.pack(mqtt.PUBLISH,v),v.seq)
end

--[[
函数名：mqttpubloc1data
功能  ：组包qos为1的MQTT PUBLISH报文数据
参数  ：无		
返回值：PUBLISH报文数据和报文参数
]]
function mqttpubloc1data()
	return mqtt.pack(mqtt.PUBLISH,{qos=1,topic="/v1/device/"..misc.getimei().."/devdata",payload="loc data1"})
end

--[[
函数名：loc1snd
功能  ：发送qos为1的MQTT PUBLISH报文
参数  ：无		
返回值：无
]]
function loc1snd()
	mqttsnd("MQTTPUBLOC1")
end

--[[
函数名：snd
功能  ：调用发送接口发送数据
参数  ：
        data：发送的数据，在发送结果事件处理函数ntfy中，会赋值到item.data中
		para：发送的参数，在发送结果事件处理函数ntfy中，会赋值到item.para中 
返回值：调用发送接口的结果（并不是数据发送是否成功的结果，数据发送是否成功的结果在ntfy中的SEND事件中通知），true为成功，其他为失败
]]
function snd(data,para)
	return linkapp.scksnd(SCK_IDX,data,para)
end

--mqtt应用报文表
tmqttpack =
{
	MQTTCONN = {sndpara="MQTTCONN",mqttyp=mqtt.CONNECT,mqttduptyp="CONN",mqttdatafnc=mqttconndata,sndcb=mqttconncb},
	MQTTSUB = {sndpara="MQTTSUB",mqttyp=mqtt.SUBSCRIBE,mqttduptyp="SUB",mqttdatafnc=mqttsubdata,sndcb=mqttsubcb},
	MQTTPINGREQ = {sndpara="MQTTPINGREQ",mqttyp=mqtt.PINGREQ,mqttdatafnc=mqttpingreqdata},
	MQTTDISC = {sndpara="MQTTDISC",mqttyp=mqtt.DISCONNECT,mqttdatafnc=mqttdiscdata,sndcb=mqttdiscb},
	MQTTPUBLOC0 = {sndpara="MQTTPUBLOC0",mqttyp=mqtt.PUBLISH,mqttdatafnc=mqttpubloc0data,sndcb=mqttpubloc0cb},
	MQTTPUBLOC1 = {sndpara="MQTTPUBLOC1",mqttyp=mqtt.PUBLISH,mqttdatafnc=mqttpubloc1data,sndcb=mqttpubloc1cb},
}

local function getidbysndpara(para)
	for k,v in pairs(tmqttpack) do
		if v.sndpara==para then return k end
	end
end

--[[
函数名：mqttsnd
功能  ：MQTT报文发送总接口，根据报文类型，在mqtt应用报文表中找到组包函数，然后发送数据
参数  ：
        typ：报文类型
返回值：无
]]
function mqttsnd(typ)
	if not tmqttpack[typ] then print("mqttsnd typ error",typ) return end
	local mqttyp = tmqttpack[typ].mqttyp
	local dat,para = tmqttpack[typ].mqttdatafnc()
	
	if mqttyp==mqtt.CONNECT then
		if tmqttpack[typ].mqttduptyp then mqttdup.rmv(tmqttpack[typ].mqttduptyp) end
		if not snd(dat,tmqttpack[typ].sndpara) and tmqttpack[typ].sndcb then
			tmqttpack[typ].sndcb(false,dat)
		end
	elseif mqttyp==mqtt.SUBSCRIBE then
		if tmqttpack[typ].mqttduptyp then mqttdup.rmv(tmqttpack[typ].mqttduptyp) end
		if not snd(dat,{typ=tmqttpack[typ].sndpara,val=para}) and tmqttpack[typ].sndcb then
			tmqttpack[typ].sndcb(false,para)
		end
	elseif mqttyp==mqtt.PINGREQ then
		snd(dat,tmqttpack[typ].sndpara)
	elseif mqttyp==mqtt.DISCONNECT then
		if not snd(dat,tmqttpack[typ].sndpara) and tmqttpack[typ].sndcb then
			tmqttpack[typ].sndcb(false,tmqttpack[typ].sndpara)
		end
	elseif mqttyp==mqtt.PUBLISH then
		if typ=="MQTTPUBLOC0" then
			if not snd(dat,tmqttpack[typ].sndpara) and tmqttpack[typ].sndcb then
				tmqttpack[typ].sndcb(false,tmqttpack[typ].sndpara)
			end
		elseif typ=="MQTTPUBLOC1" then
			if not snd(dat,{typ=tmqttpack[typ].sndpara,val=para}) and tmqttpack[typ].sndcb then
				tmqttpack[typ].sndcb(false,para)
			end
		end
		
	end	
end

--[[
函数名：reconn
功能  ：重连后台处理
        一个连接周期内的动作：如果连接后台失败，会尝试重连，重连间隔为RECONN_PERIOD秒，最多重连RECONN_MAX_CNT次
        如果一个连接周期内都没有连接成功，则等待RECONN_CYCLE_PERIOD秒后，重新发起一个连接周期
        如果连续RECONN_CYCLE_MAX_CNT次的连接周期都没有连接成功，则重启软件
参数  ：无
返回值：无
]]
local function reconn()
	print("reconn",reconncnt,reconning,reconncyclecnt)
	--conning表示正在尝试连接后台，一定要判断此变量，否则有可能发起不必要的重连，导致reconncnt增加，实际的重连次数减少
	if reconning then return end
	--一个连接周期内的重连
	if reconncnt < RECONN_MAX_CNT then		
		reconncnt = reconncnt+1
		link.shut()
		connect()
	--一个连接周期的重连都失败
	else
		reconncnt,reconncyclecnt = 0,reconncyclecnt+1
		if reconncyclecnt >= RECONN_CYCLE_MAX_CNT then
			dbg.restart("connect fail")
		end
		sys.timer_start(reconn,RECONN_CYCLE_PERIOD*1000)
	end
end

--[[
函数名：ntfy
功能  ：socket状态的处理函数
参数  ：
        idx：number类型，linkapp中维护的socket idx，跟调用linkapp.sckconn时传入的第一个参数相同，程序可以忽略不处理
        evt：string类型，消息事件类型
		result： bool类型，消息事件结果，true为成功，其他为失败
		item：table类型，{data=,para=}，消息回传的参数和数据，目前只是在SEND类型的事件中用到了此参数，例如调用linkapp.scksnd时传入的第2个和第3个参数分别为dat和par，则item={data=dat,para=par}
返回值：无
]]
function ntfy(idx,evt,result,item,rspstr)
	print("ntfy",evt,result,item)
	--连接结果
	if evt == "CONNECT" then
		reconning = false
		--连接成功
		if result then
			reconncnt,reconncyclecnt,linksta,rcvs = 0,0,true,""
			--停止重连定时器
			sys.timer_stop(reconn)
			--发送mqtt connect请求
			mqttsnd("MQTTCONN")			
		--连接失败
		else
			--5秒后重连
			sys.timer_start(reconn,RECONN_PERIOD*1000)
		end	
	--数据发送结果
	elseif evt == "SEND" then
		if not result and rspstr and smatch(rspstr,"ERROR") then
			link.shut()
		else
			if item.para then
				if item.para=="MQTTDUP" then
					mqttdupcb(result,item.data)
				else
					local id = getidbysndpara(type(item.para) == "table" and item.para.typ or item.para)
					local val = type(item.para) == "table" and item.para.val or item.data
					print("item.para",type(item.para) == "table",type(item.para) == "table" and item.para.typ or item.para,id)
					if id and tmqttpack[id].sndcb then tmqttpack[id].sndcb(result,val) end
				end
			end
		end
	--连接被动断开
	elseif (evt == "STATE" and result == "CLOSED") or evt == "DISCONNECT" then
		sys.timer_stop(pingreq)
		sys.timer_stop(loc0snd)
		sys.timer_stop(loc1snd)
		mqttdup.rmvall()
		rcvs,linksta,mqttconn = ""
		reconn()			
	end
	--其他错误处理，断开数据链路，重新连接
	if smatch((type(result)=="string") and result or "","ERROR") then
		link.shut()
	end
end

--[[
函数名：connack
功能  ：处理服务器下发的MQTT CONNACK报文
参数  ：
        packet：解析后的报文格式，table类型{suc=是否连接成功}
返回值：无
]]
local function connack(packet)
	print("connack",packet.suc)
	if packet.suc then
		mqttconn = true
		mqttdup.rmv(tmqttpack["MQTTCONN"].mqttduptyp)
		
		--订阅主题
		mqttsnd("MQTTSUB")
	end
end

--[[
函数名：suback
功能  ：处理服务器下发的MQTT SUBACK报文
参数  ：
        packet：解析后的报文格式，table类型{seq=对应的SUBSCRIBE报文序列号}
返回值：无
]]
local function suback(packet)
	print("suback",common.binstohexs(packet.seq))
	mqttdup.rmv(tmqttpack["MQTTSUB"].mqttduptyp,nil,packet.seq)
	loc0snd()
	loc1snd()
end

--[[
函数名：puback
功能  ：处理服务器下发的MQTT PUBACK报文
参数  ：
        packet：解析后的报文格式，table类型{seq=对应的PUBLISH报文序列号}
返回值：无
]]
local function puback(packet)	
	local typ = mqttdup.getyp(packet.seq) or ""
	print("puback",common.binstohexs(packet.seq),typ)
	mqttdup.rmv(nil,nil,packet.seq)
end

--[[
函数名：svrpublish
功能  ：处理服务器下发的MQTT PUBLISH报文
参数  ：
        mqttpacket：解析后的报文格式，table类型{qos=,topic,seq,payload}
返回值：无
]]
local function svrpublish(mqttpacket)
	print("svrpublish",mqttpacket.topic,mqttpacket.seq,mqttpacket.payload)	
	if mqttpacket.qos == 1 then snd(mqtt.pack(mqtt.PUBACK,mqttpacket.seq)) end	
end

--[[
函数名：pingrsp
功能  ：处理服务器下发的MQTT PINGRSP报文
参数  ：无
返回值：无
]]
local function pingrsp()
	sys.timer_stop(disconnect)
end

--服务器下发报文处理表
mqttcmds = {
	[mqtt.CONNACK] = connack,
	[mqtt.SUBACK] = suback,
	[mqtt.PUBACK] = puback,
	[mqtt.PUBLISH] = svrpublish,
	[mqtt.PINGRSP] = pingrsp,
}

--[[
函数名：datinactive
功能  ：数据通信异常处理
参数  ：无
返回值：无
]]
local function datinactive()
    dbg.restart("SVRNODATA")
end

--[[
函数名：checkdatactive
功能  ：重新开始检测“数据通信是否异常”
参数  ：无
返回值：无
]]
local function checkdatactive()
	sys.timer_start(datinactive,KEEP_ALIVE_TIME*1000*3+30000) --3倍保活时间+半分钟
end

--[[
函数名：rcv
功能  ：socket接收数据的处理函数
参数  ：
        id ：linkapp中维护的socket idx，跟调用linkapp.sckconn时传入的第一个参数相同，程序可以忽略不处理
        data：接收到的数据
返回值：无
]]
function rcv(id,data)
	print("rcv",slen(data)>200 and slen(data) or common.binstohexs(data))
	sys.timer_start(pingreq,KEEP_ALIVE_TIME*1000/2)
	rcvs = rcvs..data

	local f,h,t = mqtt.iscomplete(rcvs)

	while f do
		data = ssub(rcvs,h,t)
		rcvs = ssub(rcvs,t+1,-1)
		local packet = mqtt.unpack(data)
		if packet and packet.typ and mqttcmds[packet.typ] then
			mqttcmds[packet.typ](packet)
			if packet.typ ~= mqtt.CONNACK and packet.typ ~= mqtt.SUBACK then
				checkdatactive()
			end
		end
		f,h,t = mqtt.iscomplete(rcvs)
	end
end

--[[
函数名：connect
功能  ：创建到后台服务器的连接；
        如果数据网络已经准备好，会理解连接后台；否则，连接请求会被挂起，等数据网络准备就绪后，自动去连接后台
		ntfy：socket状态的处理函数
		rcv：socket接收数据的处理函数
参数  ：无
返回值：无
]]
function connect()	
	linkapp.sckconn(SCK_IDX,linkapp.NORMAL,PROT,ADDR,PORT,ntfy,rcv)
	reconning = true
end

--[[
函数名：connect
功能  ：mqttdup中触发的重发报文发送后的异步回调
参数  ：
		result： bool类型，发送结果，true为成功，其他为失败
		v：报文数据
返回值：无
]]
function mqttdupcb(result,v)
	mqttdup.rsm(v)
end

--[[
函数名：mqttdupind
功能  ：mqttdup中触发的重发报文处理
参数  ：
		s：报文数据
返回值：无
]]
local function mqttdupind(s)
	if not snd(s,"MQTTDUP") then mqttdupcb(false,s) end
end

--[[
函数名：mqttdupfail
功能  ：mqttdup中触发的重发报文，在最大重发次数内，都发送失败的通知消息处理
参数  ：
		t：报文的用户自定义类型
		s：报文数据
返回值：无
]]
local function mqttdupfail(t,s)
    
end

--mqttdup重发消息处理函数表
local procer =
{
	MQTT_DUP_IND = mqttdupind,
	MQTT_DUP_FAIL = mqttdupfail,
}
--注册消息的处理函数
sys.regapp(procer)

connect()
checkdatactive()
