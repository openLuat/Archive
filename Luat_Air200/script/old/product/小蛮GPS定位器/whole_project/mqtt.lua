--[[
模块名称：MQTT协议
模块功能：MQTT封包，拆包
模块最后修改时间：2017.02.13
]]

--定义模块,导入依赖库
module(...,package.seeall)
local lpack = require"pack"
require"mqttdup"

--加载常用的全局函数至本地
local slen,sbyte,ssub,sgsub,schar,srep,smatch,sgmatch = string.len,string.byte,string.sub,string.gsub,string.char,string.rep,string.match,string.gmatch
--命令消息
CONNECT,CONNACK,PUBLISH,PUBACK,PUBREC,PUBREL,PUBCOMP,SUBSCRIBE,SUBACK,UNSUBSCRIBE,UNSUBACK,PINGREQ,PINGRSP,DISCONNECT = 1,2,3,4,5,6,7,8,9,10,11,12,13,14

local PRONAME,PROVER,CLEANSESS = "MQIsdp",3,1

local seq = 1

local function print(...)
	_G.print("mqtt",...)
end

--[[
函数名：encutf8
功能  ：将字符串转成UTF8编码
参数  ：s
返回值：UTF8编码
]]
local function encutf8(s)
	if not s then return "" end
	return lpack.pack(">HA",slen(s),s)
end

--[[
函数名：enclen
功能  ：长度编码
参数  ：s
返回值：
]]
local function enclen(s)
	if not s or slen(s) == 0 then return schar(0) end
	local ret,len,digit = "",slen(s)
	repeat
		digit = len % 128
		len = len / 128
		if len > 0 then
			digit = bit.bor(digit,0x80)
		end
		ret = ret..schar(digit)
	until (len <= 0)
	return ret
end

--[[
函数名：declen
功能  ：长度解码
参数  ：s
返回值：
]]
local function declen(s)
	local i,value,multiplier,digit = 1,0,1 
	repeat
		if i > slen(s) then return end
		digit = sbyte(s,i) 
		value = value + bit.band(digit,127)*multiplier
		multiplier = multiplier * 128
		i = i + 1
	until (bit.band(digit,128) == 0)
	return true,value,i-1
end

--[[
函数名：getseq
功能  ：获取序列号
参数  ：无
返回值：序列号（1-0xFFFE）
]]
local function getseq()
	local s = seq
	seq = (seq+1)%0xFFFF
	if seq == 0 then seq = 1 end
	return lpack.pack(">H",s)
end

function iscomplete(s)
	local i,typ,flg,len,cnt
	for i=1,slen(s) do
		typ = bit.band(bit.rshift(sbyte(s,i),4),0x0f)
		--print("typ",typ)
		if typ >= CONNECT and typ <= DISCONNECT then
			flg,len,cnt = declen(ssub(s,i+1,-1))
			--print("f",flg,len,cnt,(slen(ssub(s,i+1,-1))-cnt))
			if flg and cnt <= 4 and len <= (slen(ssub(s,i+1,-1))-cnt) then
				return true,i,i+cnt+len,typ,len
			end
		end		
	end
end

--[[
函数名：pack
功能  ：MQTT报文封包
参数  ：s
返回值：解析成功：packet，解析失败：nil
]]
function pack(typ,...)
	local para = {}
    --[[
    函数名：connect
    功能：客户端请求连接服务器
    参数  ：alive,id,user,pwd
    返回值：MQTT报文
    ]]
	local function connect(alive,id,user,pwd)
		return lpack.pack(">bAbbHAAA",CONNECT*16,encutf8(PRONAME),PROVER,(user and 1 or 0)*128+(pwd and 1 or 0)*64+CLEANSESS*2,alive,encutf8(id),encutf8(user),encutf8(pwd))
	end
	
    --[[
    函数名：subscribe
    功能：客户端订阅主题
    参数  ：p
    返回值：MQTT报文
    ]]
	local function subscribe(p)
		para.dup,para.topic = true,p.topic
		para.seq = p.seq or getseq()
		print("subscribe",p.dup,para.dup,common.binstohexs(para.seq))
		
		local s = lpack.pack("bA",SUBSCRIBE*16+(p.dup and 1 or 0)*8+2,para.seq)
		for i=1,#p.topic do
			s = s..encutf8(p.topic[i]).."\1"
		end
		return s
	end
	
    --[[
    函数名：publish
    功能：客户端发送PUBLISH 消息给服务器
    参数  ：p
    返回值：MQTT报文
    ]]
	local function publish(p)
		para.dup,para.topic,para.payload,para.qos,para.retain = true,p.topic,p.payload,p.qos,p.retain
		para.seq = p.seq or getseq()
		--print("publish",p.dup,para.dup,common.binstohexs(para.seq))
		
		return lpack.pack("bAAA",PUBLISH*16+(p.dup and 1 or 0)*8+(p.qos or 0)*2+(p.retain and 1 or 0)*1,encutf8(p.topic),((p.qos or 0)>0 and para.seq or ""),p.payload)
	end
	
    --[[
    函数名：puback
    功能：发布确认
    参数  ：seq
    返回值：MQTT报文
    ]]
	local function puback(seq)
		return schar(PUBACK*16)..seq
	end
	
    --[[
    函数名：pingreq
    功能：询问服务器：“你还活着吗”？
    参数  ：无
    返回值：MQTT报文
    ]]
	local function pingreq()
		return schar(PINGREQ*16)
	end
	
    --[[
    函数名：disconnect
    功能：断开连接通知
    参数  ：无
    返回值：MQTT报文
    ]]
	local function disconnect()
		return schar(DISCONNECT*16)
	end

	local procer =
	{
		[CONNECT] = connect,
		[SUBSCRIBE] = subscribe,
		[PUBLISH] = publish,
		[PUBACK] = puback,
		[PINGREQ] = pingreq,
		[DISCONNECT] = disconnect,
	}

	local s = procer[typ](...)
	s = ssub(s,1,1)..enclen(ssub(s,2,-1))..ssub(s,2,-1)
	print("pack",typ,common.binstohexs(s))
	return s,para
end

--[[
函数名：unpack
功能  ：MQTT报文解析
参数  ：s
返回值：解析成功：packet，解析失败：nil
]]
function unpack(s)
	local packet = {}

    --[[
    函数名：connack
    功能  ：连接确认
    参数  ：d
    返回值：true
    ]]
	local function connack(d)
		print("connack",common.binstohexs(d))
		packet.suc = (sbyte(d,2)==0)
		return true
	end
	
    --[[
    函数名：suback
    功能  ：订阅确认
    参数  ：d
    返回值：true
    ]]
	local function suback(d)
		print("suback",common.binstohexs(d))
		if slen(d) < 2 then return end
		packet.seq = ssub(d,1,2)
		return true
	end
	
    --[[
    函数名：suback
    功能  ：发布确认
    参数  ：d
    返回值：true
    ]]
	local function puback(d)
		print("puback",common.binstohexs(d))
		if slen(d) < 2 then return end
		packet.seq = ssub(d,1,2)
		return true
	end
	
    --[[
    函数名：publish
    功能  ：发布消息
    参数  ：d
    返回值：true
    ]]
	local function publish(d)
		print("publish",common.binstohexs(d))
		if slen(d) < 6 then return end
		local _,tplen = lpack.unpack(ssub(d,1,2),">H")
		if slen(d) < tplen + 5 then return end
		packet.topic = ssub(d,3,2+tplen)
		local pay = 3
		if packet.qos > 0 then
			packet.seq = ssub(d,tplen+3,tplen+4)
			pay = 5
		end
		packet.payload = ssub(d,tplen+pay,-1)
		return true
	end
	
	local function empty()
		return true
	end

	local procer =
	{
		[CONNACK] = connack,
		[SUBACK] = suback,
		[PUBACK] = puback,
		[PUBLISH] = publish,
		[PINGRSP] = empty,
	}
	local d1,d2,d3,typ,len = iscomplete(s)	
	if not procer[typ] then print("unpack unknwon typ",typ) return end
	packet.typ = typ
	packet.qos = bit.rshift(bit.band(sbyte(s,1),0x06),1)
	print("unpack",typ,packet.qos,common.binstohexs(s))
	return procer[typ](ssub(s,slen(s)-len+1,-1)) and packet or nil
end
