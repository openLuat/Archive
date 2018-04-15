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
module(...)

--加载常用的全局函数至本地
local ipairs,pairs,print,unpack,type = base.ipairs,base.pairs,base.print,base.unpack,base.type
local req = ril.request

--底层通话模块是否准备就绪，true就绪，false或者nil未就绪
local ccready = true

--记录来电号码保证同一电话多次振铃只提示一次
local incoming_num = nil
--紧急号码表
local emergency_num = {"112", "911", "000", "08", "110", "119", "118", "999"}
--通话列表
local clcc,clccold,disc,chupflag = {},{},{},0
--状态变化通知回调
local usercbs = {}


--[[
函数名：print
功能  ：打印接口，此文件中的所有打印都会加上cc前缀
参数  ：无
返回值：无
]]
local function print(...)
	base.print("cc",...)
end

--[[
函数名：dispatch
功能  ：执行每个内部消息对应的用户回调
参数  ：
		evt：消息类型
		para：消息参数
返回值：无
]]
local function dispatch(evt,para)
	local tag = string.match(evt,"CALL_(.+)")
	if usercbs[tag] then usercbs[tag](para) end
end

--[[
函数名：regcb
功能  ：注册一个或者多个消息的用户回调函数
参数  ：
		evt1：消息类型，目前仅支持"READY","INCOMING","CONNECTED","DISCONNECTED"
		cb1：消息对应的用户回调函数
		...：evt和cb成对出现
返回值：无
]]
function regcb(evt1,cb1,...)
	usercbs[evt1] = cb1
	local i
	for i=1,arg.n,2 do
		usercbs[unpack(arg,i,i)] = unpack(arg,i+1,i+1)
	end
end

--[[
函数名：deregcb
功能  ：撤销注册一个或者多个消息的用户回调函数
参数  ：
		evt1：消息类型，目前仅支持"READY","INCOMING","CONNECTED","DISCONNECTED"
		...：0个或者多个evt
返回值：无
]]
function deregcb(evt1,...)
	usercbs[evt1] = nil
	local i
	for i=1,arg.n do
		usercbs[unpack(arg,i,i)] = nil
	end
end

--[[
函数名：isemergencynum
功能  ：检查号码是否为紧急号码
参数  ：
		num：待检查号码
返回值：true为紧急号码，false不为紧急号码
]]
local function isemergencynum(num)
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
	print("clearincomingflag")
	incoming_num = nil
end

--[[
函数名：clearchupflag
功能  ：清除来电标志
参数  ：无
返回值：无
]]
local function clearchupflag()
    print("clearchupflag")
    chupflag = 0
end

--[[
函数名：qrylist
功能  ：查询通话列表
参数  ：无
返回值：无
]]
local function qrylist()
	print("qrylist")
    clcc = {}
    req("AT+CLCC")
end

--[[
函数名：FindCcById
功能  ：通过id查询通话
参数  ：
    id: 通话id值
    cctb: 通话列表
返回值：通话
]]
function FindCcById(id,cctb)  
	print("FindCcById")
    for k,v in pairs(cctb) do
		print(v.id,id,cctb[k])
	    if v.id == id then
	        return cctb[k]
	    end
    end
  
    return nil
end

local checkclcc=true
local function proclist()
    print("proclist",#clccold,#clcc)
    local k,v,isactive,cc,res,hasincoming

    if #clccold == 0 then
	    clccold = clcc
	    res = true--return
    end
    for k,v in pairs(clcc) do
		print("clcc",v.dir,v.sta,incoming_num,v.num)
	    if v.dir == "1" and (v.sta == "4" or v.sta == "5") and ((incoming_num and incoming_num ==v.num) or incoming_num==nil) then
            if incoming_num==nil then incoming_num=v.num end
            cc = FindCcById(v.id,clccold)
	        if not res and cc and cc.num ==v.num and (cc.sta == "4" or cc.sta == "5") then
                print("ljdcc proclist invalid CALL_INCOMING:",incoming_num,cc.sta,v.sta)
	        else
		        print("ljdcc proclist CALL_INCOMING:",incoming_num,#clccold,v.id)
		        if res then
                    dispatch("CALL_INCOMING",incoming_num,clccold,v.id)
                else
                    hasincoming={incoming_num,clccold,v.id}
                end
	        end
	    end
    end
    if res then return end
    for k,v in pairs(clccold) do
		print("clccold",v.id)
	    cc = FindCcById(v.id,clcc)
	    if cc == nil then
	        if #clccold>0 then
	            if #clccold>1 and checkclcc then
	                qrylist()
	                checkclcc = false
	                if hasincoming then--存在这一的情况：刚呼出的同时有呼入，呼出失败，第一次clcc中id为1的通话是呼出，第二次clcc中id为1的是呼入，第二次clcc结果锅里中既要处理disc消息，也要处理incoming消息，优先处理disc消息
                        print("ljdcc real dispatch incom ",hasincoming[1],hasincoming[2],hasincoming[3])
                        dispatch("CALL_INCOMING",hasincoming[1],hasincoming[2],hasincoming[3])
                    end
	                return
	            else
    		        print("ljdcc proclist CALL_DISCONNECTED",disc[1] or "invalid reason")
    		        dispatch("CALL_DISCONNECTED",disc[1] or "invalid reason",clccold,v.id)
    		        chupflag,disc,checkclcc,incoming_num = 1,{},true
    		        sys.timer_start(clearchupflag,2000)
		        end
	        end
	  
	    else
	        if cc.dir == v.dir and cc.num ==v.num and cc.mode ==v.mode then
                print("ljdcc proclist CALL_CONNECTED = ",(cc.sta =="0" and v.sta ~="0"),cc.sta,v.sta)
			    if cc.sta =="0" and v.sta ~="0" then
			        dispatch("CALL_CONNECTED",clccold,v.id)
			    end
	        else
	            dispatch("CALL_DISCONNECTED",disc[1] or "invalid reason",clccold,v.id)
	            chupflag,disc,checkclcc,incoming_num = 1,{},true
                sys.timer_start(clearchupflag,2000)
		        print("ljdcc maybe someting err , cc.dir:",cc.dir,"v.dir:",v.dir,"cc.num:",cc.num,"v.num:",v.num,"cc.mode:",cc.mode,"v.mode:",v.mode) 	    
	        end
	    end
    end
    
    --存在这一的情况：刚呼出的同时有呼入，呼出失败，第一次clcc中id为1的通话是呼出，第二次clcc中id为1的是呼入，第二次clcc结果锅里中既要处理disc消息，也要处理incoming消息，优先处理disc消息
    if hasincoming then
        print("ljdcc real dispatch incom ",hasincoming[1],hasincoming[2],hasincoming[3])
        dispatch("CALL_INCOMING",hasincoming[1],hasincoming[2],hasincoming[3])
    end
  
    clccold = clcc
end

local function discevt(reason)
	pm.sleep("cc")
	table.insert(disc,reason)
	print("ljdcc discevt reason:",reason,#clccold,#clcc)
	--dispatch("CALL_DISCONNECTED",reason)
	qrylist()
end

function anycallexist()
	return #clccold>0
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

	if (ccready == false or net.getstate() ~= "REGISTERED") and not isemergencynum(number) then
		return false
	end

	pm.wake("cc")
	req(string.format("%s%s;","ATD",number),nil,nil,delay)
	qrylist()

	return true
end

function dropcallbyarg(statb,dir)
    if type(statb) ~= "table" or #statb==0 then
	    print("ljdcc dropcallbyarg err statb ind")
	    return
    end
    print("ljdcc dropcallbyarg ",#statb,dir,#clccold)
    for k,v in pairs(clccold) do
		print(dir,v.dir)
	    if v.dir==dir then
	        for i=1,#statb do
		        print("ljdcc dropcallbyarg ",statb[i],v.sta)
		        if v.sta == statb[i] then
		            req("AT+CHLD=1"..v.id)
		            print("ljdcc hangup:",v.num) 
					return true
		        end
	        end
	    end 
    end
end

--[[
函数名：hangup
功能  ：主动挂断所有通话
参数  ：无
返回值：无
]]
function hangup()
	--aud.stop()
	if #clccold==1 then
	    req("AT+CHUP")
	else
	    for k,v in pairs(clccold) do
	        if v.sta == "0" then 
		        req("AT+CHLD=1"..v.id)
		        print("ljdcc hangup:",v.num) 
		        break 
	        end
	    end
	end
end

--[[
函数名：accept
功能  ：接听来电
参数  ：无
返回值：无
]]
function accept()
	--aud.stop()
	req("ATA")
	pm.wake("cc")
end

--[[
函数名：transvoice
功能  ：通话中发送声音到对端,必须是12.2K AMR格式
参数  ：
返回值：true为成功，false为失败
]]
function transvoice(data,loop,loop2)
	local f = io.open("/RecDir/rec000","wb")

	if f == nil then
		print("transvoice:open file error")
		return false
	end

	-- 有文件头并且是12.2K帧
	if string.sub(data,1,7) == "#!AMR\010\060" then
	-- 无文件头且是12.2K帧
	elseif string.byte(data,1) == 0x3C then
		f:write("#!AMR\010")
	else
		print("transvoice:must be 12.2K AMR")
		return false
	end

	f:write(data)
	f:close()

	req(string.format("AT+AUDREC=%d,%d,2,0,50000",loop2 == true and 1 or 0,loop == true and 1 or 0))

	return true
end

--[[
函数名：dtmfdetect
功能  ：设置dtmf检测是否使能以及灵敏度
参数  ：
		enable：true使能，false或者nil为不使能
		sens：灵敏度，默认3，最灵敏为1
返回值：无
]]
function dtmfdetect(enable,sens)
	if enable == true then
		if sens then
			req("AT+DTMFDET=2,1," .. sens)
		else
			req("AT+DTMFDET=2,1,3")
		end
	end

	req("AT+DTMFDET="..(enable and 1 or 0))
end

--[[
函数名：senddtmf
功能  ：发送dtmf到对端
参数  ：
		str：dtmf字符串
		playtime：每个dtmf播放时间，单位毫秒，默认100
		intvl：两个dtmf间隔，单位毫秒，默认100
返回值：无
]]
function senddtmf(str,playtime,intvl)
	if string.match(str,"([%dABCD%*#]+)") ~= str then
		print("senddtmf: illegal string "..str)
		return false
	end

	playtime = playtime and playtime or 100
	intvl = intvl and intvl or 100

	req("AT+SENDSOUND="..string.format("\"%s\",%d,%d",str,playtime,intvl))
end

local dtmfnum = {[71] = "Hz1000",[69] = "Hz1400",[70] = "Hz2300"}

--[[
函数名：parsedtmfnum
功能  ：dtmf解码，解码后，会产生一个内部消息AUDIO_DTMF_DETECT，携带解码后的DTMF字符
参数  ：
		data：dtmf字符串数据
返回值：无
]]
local function parsedtmfnum(data)
	local n = base.tonumber(string.match(data,"(%d+)"))
	local dtmf

	if (n >= 48 and n <= 57) or (n >=65 and n <= 68) or n == 42 or n == 35 then
		dtmf = string.char(n)
	else
		dtmf = dtmfnum[n]
	end

	if dtmf then
		dispatch("CALL_DTMF",dtmf)
	end
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
	--通话建立通知
	elseif data == "CONNECT" then
		qrylist()		
		dispatch("CALL_CONNECTED")
	--通话挂断通知
	elseif data == "NO CARRIER" or data == "BUSY" or data == "NO ANSWER" then
	    print("ljdcc ",data,chupflag,#clccold,#clcc)
	    if #clccold==0 and #clcc==0 then
	        return
	    end
		discevt(data)
	--来电振铃
	elseif prefix == "+CLIP" then
	    print("ljdcc CLIP CALL_INCOMING",incoming_num,"chupflag:",chupflag)
		local number = string.match(data,"\"(%+*%d*)\"",string.len(prefix)+1)
		if incoming_num ~= number then
			incoming_num = number
			if chupflag==1 then
			  sys.timer_start(qrylist,1500)
			else
			  qrylist()
			end
			--dispatch("CALL_INCOMING",number)
		end
	--通话列表信息
	elseif prefix == "+CLCC" then
		local id,dir,sta,mode,mpty,num = string.match(data,"%+CLCC:%s*(%d+),(%d),(%d),(%d),(%d),\"(%+*%d*)\"")
		if id then
		    local cc=FindCcById(id,clcc)
		    if cc== nil then
			    table.insert(clcc,{id=id,dir=dir,sta=sta,mode=mode,mpty=mpty,num=num})
			else
			    cc.dir,cc.sta,cc.mode,cc.mpty,cc.num = dir,sta,mode,mpty,num
			end		
		end
	--DTMF接收检测
	elseif prefix == "+DTMFDET" then
		parsedtmfnum(data)
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
	print("ljdcc ccrsp",prefix,cmd,success,response,intermediate)
	--拨号应答
	if prefix == "D" then
		if not success then
			discevt("CALL_FAILED")
		else
			if usercbs["ALERTING"] then sys.timer_loop_start(qrylist,1000,"MO") end
		end
	--挂断所有通话应答
	elseif prefix == "+CHUP" then
		discevt("LOCAL_HANG_UP")
	elseif prefix == "+CLCC" then
	    proclist()
    elseif prefix=='+CHLD' and (response=='ERROR' or response=='NO ANSWER') then
    	qrylist()
	--接听来电应答
	elseif prefix == "A" then
		incoming_num = nil
		qrylist()
		--dispatch("CALL_CONNECTED")
	end
end

--注册以下通知的处理函数
ril.regurc("CALL READY",ccurc)
ril.regurc("CONNECT",ccurc)
ril.regurc("NO CARRIER",ccurc)
ril.regurc("NO ANSWER",ccurc)
ril.regurc("BUSY",ccurc)
ril.regurc("+CLIP",ccurc)
ril.regurc("+CLCC",ccurc)

ril.regurc("+DTMFDET",ccurc)
--注册以下AT命令的应答处理函数
ril.regrsp("D",ccrsp)
ril.regrsp("A",ccrsp)
ril.regrsp("+CHUP",ccrsp)
ril.regrsp("+CLCC",ccrsp)
ril.regrsp("+CHLD",ccrsp)
--开启拨号音,忙音检测
req("ATX4")
--开启来电urc上报
req("AT+CLIP=1")
dtmfdetect(true)
