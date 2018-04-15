--[[
模块名称：linkair
模块功能：
模块最后修改时间：2017.02.13
]]

--定义模块,导入依赖库
module(...,package.seeall)

require"protoair"
require"sms"
require"dbg"
require"ccair"

--加载常用的全局函数至本地
local ssub,schar,smatch = string.sub,string.char,string.match
local SCK_IDX,KEEP_ALIVE_TIME = 1,720
--rests存放收到的报文
--shkcnt震动次数
local rests,shkcnt = "",0
--locshk位置是否发生移动，"SIL"未移动，"SHK"移动
local locshk,firstloc,firstgps = "SIL"
--chgalm 低电报警
--shkalm 震动报警
local chgalm,shkalm = true,true
--verno协议版本号
--mqttconnfailcnt mqttconn连接失败次数
--mqttconn true mqtt连接
local verno,mqttconnfailcnt,mqttconn = 1,0

local function print(...)
	_G.print("linkair",...)
end

--[[
函数名：getstatus
功能  ：获取设备状态
参数  ：无
返回值：设备状态表
]]
local function getstatus()
	local t = {}

	t.shake = (shkcnt > 0) and 1 or 0
	shkcnt = 0
	t.charger = chg.getcharger() and 1 or 0
	t.acc = 0
	t.gps = gps.isopen() and 1 or 0
	t.sleep = pm.isleep() and 1 or 0
	t.volt = chg.getvolt()

	return t
end

--[[
函数名：getgps
功能  ：获取gps定位信息
参数  ：无
返回值：gps定位信息表
]]
local function getgps()
	local t = {}
	print("getgps:",gps.getgpslocation(),gps.getgpscog(),gps.getgpsspd())
	t.fix = gps.isfix()
	if gps.isfix() then
		t.lng,t.lat = smatch(gps.getgpslocation(),"[EW]*,(%d+%.%d+),[NS]*,(%d+%.%d+)")
	else
		t.lng,t.lat = manage.getlastgps()
	end
	t.lng,t.lat = t.lng or "",t.lat or ""
	t.cog = gps.getgpscog()
	t.spd = gps.getgpsspd()

	return t
end

--[[
函数名：getgpstat
功能  ：获取卫星数
参数  ：无
返回值：卫星数
]]
local function getgpstat()
	local t = {}
	t.satenum = gps.getgpssatenum()
	return t
end

local function getfncpara()
	local n1 = (manage.GUARDFNC and 1 or 0) + (manage.HANDLEPWRFNC and 1 or 0)*2 + (manage.MOTORPWRFNC and 1 or 0)*4 + (manage.RMTPWRFNC and 1 or 0)*8
	n1 = n1 + (manage.BUZZERFNC and 1 or 0)*16
	local n2,n3,n4,n5,n6,n7,n8 = 0,0,0,0,0,0,0
	return pack.pack("bbbbbbbb",n1,n2,n3,n4,n5,n6,n7,n8)
end

--[[
函数名：getcellinfo
功能  ：获取小区信息
参数  ：无
返回值：小区信息
]]
 function getcellinfo()
	return smatch(net.getcellinfo(),"(%w+%.%w+%.%w+;)")
end

--[[
函数名：getcellinfoext
功能  ：获取小区扩展信息
参数  ：无
返回值：小区扩展信息
]]
 function getcellinfoext()
	return smatch(net.getcellinfoext(),"(%w+%.%w+%.%w+%.%w+%.%w+;)")
end

local tget = {
	["VERSION"] = function() return _G.VERSION end,
	PROJECTID = function() return _G.PRJID end,
	PROJECT = function() return 1 end,
	HEART = function() return nvm.get("heart") end,
	IMEI = misc.getimei,
	IMSI = sim.getimsi,
	ICCID = sim.geticcid,
	RSSI = net.getrssi,
	MNC = function() return tonumber(net.getmnc()) end,
	MCC = function() return tonumber(net.getmcc()) end,
	CELLID = function() return tonumber(net.getci(),16) end,
	LAC = function() return tonumber(net.getlac(),16) end,
	CELLINFO = getcellinfo,--net.getcellinfo,
	CELLINFOEXT = getcellinfoext,--net.getcellinfoext,
	TA = net.getta,
	STATUS = getstatus,
	GPS = getgps,
	GPSTAT = getgpstat,
	FNCPARA = getfncpara,
	MOVSTA = function() return manage.getmovsta() end
}
local function getf(id)
	assert(tget[id] ~= nil,"getf nil id:" .. id)
	return tget[id]()
end

protoair.reget(getf)

local levt,lval,lerrevt,lerrval = "","","",""

--[[
函数名：datinactive
功能  ：与后台断开连接后重启设备
参数  ：无
返回值：无
]]
local function datinactive()
    nvm.set("abnormal",true)
	--if nvm.get("workmod") == "SMS" or nvm.get("sleep") or nvm.get("gpsleep") then return end
	dbg.restart("AIRNODATA" .. ((levt ~= nil) and (",EVT=" .. levt) or "") .. ((lval ~= nil) and (",VAL=" .. lval) or "").. ((lerrevt ~= nil) and (",ERREVT=" .. lerrevt) or "") .. ((lerrval ~= nil) and (",ERRVAL=" .. lerrval) or ""))
	mqttconnfailcnt = 0
end

--[[
函数名：checkdatactive
功能  ：如果3倍KEEP_ALIVE_TIME+半分钟还没连上后台则重启设备
参数  ：无
返回值：无
]]
local function checkdatactive()
	if nvm.get("workmod") == "SMS" then return end
	sys.timer_start(datinactive,KEEP_ALIVE_TIME*1000*3+30000) --3倍心跳+半分钟
end

--[[
函数名：snd
功能  ：向后台发送报文
参数  ：data,para,pos,ins
返回值：true，发送成功，nil或false发送失败
]]
local function snd(data,para,pos,ins)
	return linkapp.scksnd(SCK_IDX,data,para,pos,ins)
end

--[[
函数名：chgalmrpt
功能  ：低电报警上报，实质只发了心跳报文，是否推送低电报警由后台决定
参数  ：无
返回值：无
]]
local function chgalmrpt()
	if nvm.get("guard") and not chg.getcharger() and chgalm then
		chgalm = false
		sys.dispatch("ALM_IND","CHG")
		heart()
		--startalmtimer("CHG",true)
	end
end

--[[
函数名：shkalmrpt
功能  ：震动报警上报，实质只发了心跳报文，是否推送震动报警由后台决定
参数  ：无
返回值：无
]]
local function shkalmrpt()
	print("shkalmrpt",nvm.get("guard"),shkalm)
	if nvm.get("guard") and shkalm then
		shkalm = false
		sys.dispatch("ALM_IND","SHK")
		heart()
		startalmtimer("SHK",true)
	end
end

--[[
函数名：almtimerfnc
功能  ：低电、震动报警定时器处理函数
参数  ：tag区分是低电报警还是震动报警
返回值：无
]]
function almtimerfnc(tag)
	print("almtimerfnc",tag)
	local val = nvm.get("guard")
	if tag == "CHG" then
		chgalm = val
		chgalmrpt()
	elseif tag == "SHK" then
		shkalm = val
	end
end

--[[
函数名：startalmtimer
功能  ：开启报警定时器
参数  ：tag区分是低电报警还是震动报警
        val ：true开启否则不开启
返回值：无
]]
function startalmtimer(tag,val)
	print("startalmtimer",tag,val,nvm.get("guard"))
	if nvm.get("guard") then
		if val then
			sys.timer_start(almtimerfnc,nvm.get("almfreq")*60000+5000,tag)
		end
	end
end

--[[
函数名：stopalmtimer
功能  ：停掉报警定时器
参数  ：tag区分是低电报警还是震动报警
返回值：无
]]
local function stopalmtimer(tag)
	print("stopalmtimer",tag)
	sys.timer_stop(almtimerfnc,tag)
end

--[[
函数名：alminit
功能  ：报警初始化，初始化低电报警，震动报警开关
参数  ：无
返回值：无
]]
local function alminit()
	print("alminit",nvm.get("guard"))
	local val = nvm.get("guard")
	chgalm,shkalm = val,val
	stopalmtimer("CHG")
	stopalmtimer("SHK")
end

--[[
函数名：qrylocfnc
功能  ：主动定位
参数  ：无
返回值：无
]]
local function qrylocfnc()
	locrpt("QRYLOC")
end

--[[
函数名：preloc
功能  ：位置上报
参数  ：无
返回值：无
]]
local function preloc()
	print("preloc",locshk)
	loc()
	--[[if locshk == "SHK" then
		local mod = nvm.get("fixmod")
		if mod == "LBS" then
			loc()
		elseif mod == "GPS" then
			loc()
		end
	end]]
end

--[[
函数名：loctimerfnc
功能  ：位置定时上报
参数  ：无
返回值：无
]]
local function loctimerfnc()
	print("loctimerfnc",locshk)
	if locshk == "SIL" then	locshk = "TMOUT" end
	preloc()
end

--[[
函数名：startloctimer
功能  ：开启位置定时上报定时器
参数  ：无
返回值：无
]]
function startloctimer()
	print("startloctimer",nvm.get("rptfreq"))
	sys.timer_start(loctimerfnc,nvm.get("rptfreq")*1000)
end

--[[
函数名：entopic
功能  ：topic封包
参数  ：t，topic类型
返回值：字符串
]]
local function entopic(t)
	return "/v"..verno.."/device/"..tget["IMEI"]().."/"..t
end

--[[
函数名：starthearttimer
功能  ：开启心跳定时上报定时器
参数  ：无
返回值：无
]]
local function starthearttimer()
	print("starthearttimer",nvm.get("heart"))
	sys.timer_start(heart,nvm.get("heart")*1000)
end

--[[
函数名：locrpt
功能  ：位置上报
参数  ：r定位类型,w wifi热点信息
返回值：无
]]
function locrpt(r,w)
	if not mqttconn then return end
	local id,mod,extyp = protoair.LBS3,r or nvm.get("fixmod")
	if mod == "LBS" then
		id = nvm.get("lbstyp")==1 and protoair.LBS1 or protoair.LBS3
	elseif mod == "GPS" then
		id = gps.isfix() and (nvm.get("gpslbsmix") and protoair.GPSLBS1 or protoair.GPS) or (nvm.get("lbstyp")==1 and protoair.LBS1 or (nvm.get("gpslbsmix") and protoair.GPSLBS1 or protoair.LBS3))
	elseif mod == "QRYLOC" then
		id = protoair.GPSLBSWIFIEXT
		extyp = protoair.QRYPOS
	elseif mod == "KEYRPT" then
		id = protoair.GPSLBSWIFIEXT
		extyp = protoair.KEYPOS
	end	
	print("locrpt",id,w,r)
	local p = {}
	p.lbs1 = tget["LAC"]().."@"..tget["CELLID"]()
	p.lbs2 = tget["CELLINFOEXT"]()
	p.gpsfix = getgps().fix
	p.gps = getgps().lng.."@"..getgps().lat	
	return snd(mqtt.pack(mqtt.PUBLISH,{topic=entopic("devdata"),payload=protoair.pack(id,w,extyp)}),{typ="MQTTPUBLOC",val={typ=id,para=p}})
end

--[[
函数名：loc
功能  ：位置上报
参数  ：r定位类型,p wifi热点信息
返回值：无
]]
function loc(r,p)
	print("loc",gps.isfix(),r,nvm.get("gpsleep"),manage.getmovsta())
	startloctimer()
	if manage.getmovsta()=="SIL" then return end  --运动状态下只报位置不报心跳，静止状态下只上报心跳，不上报位置
	if gps.isfix() then
		local t = getgps()
		if manage.isgpsmove(t.lng,t.lat) then
			return locrpt(nil,p)
		else
			locshk = "SIL"
		end
	else
		if nvm.get("lbstyp")==1 and manage.islbs1move(tostring(tget["LAC"]()),tostring(tget["CELLID"]())) then
			return locrpt(nil,p)
		elseif nvm.get("lbstyp")==2 and manage.islbs2move(tget["CELLINFOEXT"]()) then
			return locrpt(nil,p)
		else
			locshk = "SIL"
		end
	end
end

--[[
函数名：heart
功能  ：心跳报文上报
参数  ：v true强制上报心跳
返回值：无
]]
function heart(v)
	print("heart",nvm.get("gpsleep"),mqttconn,manage.getmovsta(),v)
	starthearttimer()
	if not mqttconn then return end
	if manage.getmovsta()=="SIL" or v then--运动状态下只报位置不报心跳，静止状态下只上报心跳，不上报位置
		snd(mqtt.pack(mqtt.PUBLISH,{topic=entopic("devdata"),payload=protoair.pack(protoair.HEART)}),{typ="MQTTPUBHEART"})
	end
end

--[[
函数名：disconnect
功能  ：断开连接
参数  ：无
返回值：无
]]
local function disconnect()
	snd(mqtt.pack(mqtt.DISCONNECT),"MQTTDISC")
end

--[[
函数名：pingreq
功能  ：检测mqtt是否断开连接
参数  ：无
返回值：无
]]
local function pingreq()
	snd(mqtt.pack(mqtt.PINGREQ))
	if not sys.timer_is_active(disconnect) then
		sys.timer_start(disconnect,(KEEP_ALIVE_TIME+30)*1000)
	end
end

--[[
函数名：enrptpara
功能  ：将需要上报的参数封包
参数  ：无
返回值：字符串
]]
local function enrptpara(typ)
	local function enunsignshort(p)
		return pack.pack(">H",nvm.get(p))
	end	
	
	local proc =
	{
		rptfreq = {k=protoair.RPTFREQ,v=enunsignshort},
		almfreq = {k=protoair.ALMFREQ,v=enunsignshort},
		guard = {v=function() return schar(nvm.get("guard") and protoair.GUARDON or protoair.GUARDOFF) end},		
		fixmod = {v=function() return pack.pack("bb",protoair.FIXMOD,(nvm.get("fixmod")=="LBS") and 0 or ((nvm.get("fixmod")=="GPS") and 1 or 2)) end},
		workmod = {v=function() return pack.pack("bb",protoair.FIXMOD,(nvm.get("workmod")=="SMS") and 3 or ((nvm.get("workmod")=="GPS") and 4 or ((nvm.get("workmod")=="PWRGPS") and 5 or (nvm.get("workmod")=="LONGPS") and 6 or 7))) end},
	}
	if not proc[typ] then return "" end
	local ret = ""
	if proc[typ].k then ret = schar(proc[typ].k) end
	if proc[typ].v then ret = ret..proc[typ].v(typ) end
	return ret
end

local tpara,tparapend =
{
	"rptfreq","almfreq","guard","fixmod","workmod"
},{}

--[[
函数名：rptpara
功能  ：上报参数
参数  ：无
返回值：无
]]
local function rptpara(typ)
	if nvm.get("workmod") == "SMS" and typ~="workmod" then return end
	print("rptpara",typ,tget["IMEI"](),#tparapend)
	if typ then
		if tget["IMEI"]() ~= "" then
			for k,v in pairs(tpara) do
				if typ == v then
					mqttdup.rmv("PUBRPTPARA"..v)
					local dat,para = mqtt.pack(mqtt.PUBLISH,{qos=1,topic=entopic("devpararpt"),payload=protoair.pack(protoair.RPTPARA,enrptpara(v))})
					para.usr = v
					if not snd(dat,{typ="MQTTPUBRPTPARA",val=para},nil,true) then
						mqttpubrptparacb(para)
					else
						return true
					end
				end
			end
		else
			local fnd
			for i=1,#tparapend do
				if tparapend[i] == typ then fnd = true break end
			end
			if not fnd then
				for k,v in pairs(tpara) do
					if v == typ then tparapend[#tparapend+1] = typ end
				end
			end
		end
	else
		for i=1,#tparapend do
			if tparapend[i]~="workmod" then
				rptpara(tparapend[i])
			end
		end
		tparapend = {}
	end
end

--[[
函数名：connect
功能  ：连接后台
参数  ：cause
返回值：无
]]
function connect(cause)
	if nvm.get("workmod") == "SMS" then return end
	linkapp.sckconn(SCK_IDX,cause,nvm.get("prot"),nvm.get("addr"),nvm.get("port"),ntfy,rcv)
end

--reconntimes重连次数
local reconntimes = 0
--[[
函数名：reconn
功能  ：重连后台
参数  ：无
返回值：无
]]
local function reconn()
	if nvm.get("workmod") == "SMS" or nvm.get("gpsleep") then
		reconntimes = 0
		return
	end
	if reconntimes < 3 then
		reconntimes = reconntimes+1
		link.shut()
		connect(linkapp.NORMAL)
	else
		reconntimes = 0
		sys.timer_start(reconn,300000)
	end
end

--[[
函数名：connack
功能  ：连后台ack
参数  ：packet，连接后台结果
返回值：无
]]
local function connack(packet)
	print("connack",packet.suc)
	if packet.suc then
		mqttconn = true
		mqttdup.rmv("CONN")
		if nvm.get("workmodpend") then rptpara("workmod") end
		if nvm.get("gpsleep") or nvm.get("workmod")=="PWRGPS" or nvm.get("workmod")=="PWOFF" then
			heart()
		end
			if not sys.timer_is_active(heart) then starthearttimer() end
			mqttdup.rmv("SUB")
			local dat,para = mqtt.pack(mqtt.SUBSCRIBE,{topic={entopic("devparareq/+"),entopic("deveventreq/+")}})
			--print("connack",para.dup,common.binstohexs(para.seq))
			if not snd(dat,{typ="MQTTSUB",val=para}) then mqttsubcb(para) end
		--end		
	else
		mqttconnfailcnt = mqttconnfailcnt + 1
	end
end

--[[
函数名：devinfo
功能  ：发送终端信息报文
参数  ：无
返回值：无
]]
function devinfo()
	if not mqttconn then return end
	snd(mqtt.pack(mqtt.PUBLISH,{topic=entopic("devdata"),payload=protoair.pack(protoair.INFO)}),{typ="MQTTPUBINFO"})
end

--[[
函数名：suback
功能  ：订阅报文ack
参数  ：packet订阅结果
返回值：无
]]
local function suback(packet)
	print("suback",common.binstohexs(packet.seq))
	mqttdup.rmv("SUB",nil,packet.seq)
	
	devinfo()
	
	if firstloc then
		preloc()
	else
		loc()
	end	
end

--[[
函数名：puback
功能  ：publish ack
参数  ：packet
返回值：无
]]
local function puback(packet)	
	local typ = mqttdup.getyp(packet.seq) or ""
	print("puback",common.binstohexs(packet.seq),typ)
	if typ == "PUBDEVEVENTRSP"..protoair.RESET then
        nvm.set("abnormal",false)
		sys.timer_start(dbg.restart,2000,"AIRTCPSVR")  
	elseif typ == "PUBDEVEVENTRSP"..protoair.POWEROFF then
		sys.timer_start(sys.dispatch,3000,"REQ_PWOFF","SVR")
	elseif typ=="PUBRPTPARA".."workmod" then
		nvm.set("workmodpend",false)
	elseif smatch(typ,"^PUBQRYRCD&") then
        local seq,cur = smatch(typ,"PUBQRYRCD&(%d+)!(%d+)")
        sys.dispatch("SND_QRYRCD_CNF",true,seq,cur)
	end
	mqttdup.rmv(nil,nil,packet.seq)
end

--[[
函数名：fixmodpara
功能  ：工作模式参数的修改
参数  ：p 工作模式值
返回值：true
]]
local function fixmodpara(p)
	return nvm.set("workmod",p.val==3 and "SMS" or (p.val==4 and "GPS" or (p.val==5 and "PWRGPS" or (p.val==6 and "LONGPS" or "PWOFF"))),"SVR")
end

--[[
函数名：setpara
功能  ：修改参数
参数  ：packet 报文
返回值：true
]]
local function setpara(packet)
	local procer,result = {
		[protoair.RPTFREQ] = "rptfreq",
		[protoair.ALMFREQ] = "almfreq",		
		[protoair.GUARDON] = {k="guard",v=true},
		[protoair.GUARDOFF] = {k="guard",v=false},		
		[protoair.FIXMOD] = fixmodpara,		
	}
	if procer[packet.cmd] then
		local typ = type(procer[packet.cmd])
		if typ == "function" then
			result = procer[packet.cmd](packet)
		elseif typ == "table" then
			if type(procer[packet.cmd].k) == "function" then
				result = procer[packet.cmd].k(packet,procer[packet.cmd].v)
			else
				result = nvm.set(procer[packet.cmd].k,procer[packet.cmd].v,"SVR")
			end
		else
			result = nvm.set(procer[packet.cmd],packet.val,"SVR")
		end		
	end
	mqttdup.rmv("PUBSETPARARSP"..packet.cmd)
	local dat,para = mqtt.pack(mqtt.PUBLISH,{qos=1,topic=entopic("devpararsp/"..(smatch(packet.topic,"devparareq/(.+)") or "")),payload=protoair.pack(protoair.SETPARARSP,packet.cmd,schar(result and 1 or 0))})
	para.usr = packet.cmd
	if not snd(dat,{typ="MQTTPUBSETPARARSP",val=para}) then mqttpubsetpararspcb(para) end
end

--[[
函数名：sendsms
功能  ：发送短信
参数  ：packet 报文
返回值：无
]]
local function sendsms(packet)
	if packet.coding == "UCS2" then
		local num,i = ""
		for i=1,#packet.num do
			print("sendsms",packet.num[i])
			if string.len(packet.num[i]) >= 5 and not string.match(num,packet.num[i]) then
				num = num..packet.num[i].."@"
			end
		end
		print("sendsms1",num)
		for i in string.gmatch(num,"(%d+)@") do
			sms.send(i,common.binstohexs(packet.data))
		end		
	end
end

--[[
函数名：dial
功能  ：拨打电话
参数  ：packet 报文
返回值：无
]]
local function dial(packet)
	while #packet.num > 0 do
		sys.dispatch("CCAPP_ADD_NUM",table.remove(packet.num,1))
	end
	sys.dispatch("CCAPP_DIAL_NUM")
end

--[[
函数名：deveventrsp
功能  ：被动处理事件的应答
参数  ：packet 报文
返回值：无
]]
local function deveventrsp(packet)
	mqttdup.rmv("PUBDEVEVENTRSP"..packet.id)
	local dat,para = mqtt.pack(mqtt.PUBLISH,{qos=1,topic=entopic("deveventrsp/"..(smatch(packet.topic,"deveventreq/(.+)") or "")),payload=protoair.pack(protoair.DEVEVENTRSP,packet.id,schar(1))})
	para.usr = packet.id
	if not snd(dat,{typ="MQTTPUBDEVEVENTRSP",val=para}) then mqttpubdeveventrspcb(para) end
end

--[[
函数名：qryloc
功能  ：主动定位
参数  ：packet 报文
返回值：无
]]
local function qryloc(packet)
	--[[if not gpsapp.isactive(gpsapp.TIMERORSUC,{cause="QRYLOC"}) then
		gpsapp.open(gpsapp.TIMERORSUC,{cause="QRYLOC",val=120,cb=qrylocfnc})
	end]]
	deveventrsp(packet)
	qrylocfnc()
	sys.dispatch("SVR_QRY_LOC_IND")	
end

--[[
函数名：restore
功能  ：恢复出厂设置
参数  ：packet 报文
返回值：无
]]
local function restore(packet)
	sys.dispatch("SVR_RESTORE_IND")
	deveventrsp(packet)
end

--[[
函数名：devqrypararsp
功能  ：查询参数应答
参数  ：packet 报文
返回值：无
]]
local function devqrypararsp(packet,rsp)
	mqttdup.rmv("PUBDEVEVENTRSP"..packet.id..packet.val)
	local dat,para = mqtt.pack(mqtt.PUBLISH,{qos=1,topic=entopic("deveventrsp/"..smatch(packet.topic,"deveventreq/(.+)")),payload=protoair.pack(protoair.DEVEVENTRSP,packet.id,schar(packet.val)..rsp)})
	para.usr = packet.id..packet.val
	if not snd(dat,{typ="MQTTPUBDEVEVENTRSP",val=para}) then mqttpubdeveventrspcb(para) end
end

--[[
函数名：devqrypara
功能  ：查询参数
参数  ：packet 报文
返回值：无
]]
local function qrypara(packet)
	local procer,rsp = {
		[protoair.VER] = function() return _G.PROJECT.."_".._G.VERSION end,
	}
	if procer[packet.val] then
		rsp = procer[packet.val]()
	end
	if rsp then devqrypararsp(packet,rsp) end
end

--[[
函数名：qryrcd
功能  ：发送拾音请求
参数  ：packet 报文
返回值：无
]]
local function qryrcd(packet)
	print("qryrcd",packet.val,packet.typ)
	deveventrsp(packet)
	sys.dispatch("QRY_RCD_IND",packet.val*1000,packet.typ)
end

--[[
函数名：sndqryrcdreq
功能  ：发送录音文件
参数  ：s报文序列号,
        w发送方式0：录完再发 1：边录边发,
        t报文总条数,
        c当前报文索引,
        tm音频文件类型,
        d当前报文内容
返回值：无
]]
local function sndqryrcdreq(s,w,t,c,tm,d)
	if not (s and w and t and c and tm and d) then sys.dispatch("SND_QRYRCD_CNF",false,s,c) return end
	mqttdup.rmv("PUBQRYRCD&"..s.."!"..c)
	local dat,para = mqtt.pack(mqtt.PUBLISH,{qos=1,topic=entopic("soundrecord"),payload=protoair.pack(protoair.QRYRCDREQ,s,w,t,c,0,tm,d)})
	para.usr = s.."!"..c
	if not snd(dat,{typ="MQTTPUBQRYRCD",val=para}) then 
	    sys.dispatch("SND_QRYRCD_CNF",false,s,c) 
    end
end

--[[
函数名：smsrptreq
功能  ：将收到的短信消息上报后台
参数  ：num号码,
        data短信内容
        datetime收到短信的时间
返回值：true
]]
local function smsrptreq(num,data,datetime)
    mqttdup.rmv("PUBSMSRPTREQ")
    local dat,para = mqtt.pack(mqtt.PUBLISH,{qos=1,topic=entopic("devdata"),payload=protoair.pack(protoair.SMSRPT,datetime,num,data)})
    if not snd(dat,{typ="MQTTPUBSMSRPTREQ",val=para}) then 
        sys.dispatch("SND_SMSRPT_CNF",false) 
    else
        sys.dispatch("SND_SMSRPT_CNF",true)
    end
    return true
end

--[[
函数名：ccrptreq
功能  ：将来电或去电上报后台
参数  ：cctyp 电话类型，0：来电，1：去电
        num号码,
        starttm 来电或去电的起始时间
        ringtm响铃时间
        totaltm通话总时长
返回值：true
]]
local function ccrptreq(cctyp,ccnum,starttm,ringtm,totaltm)
    mqttdup.rmv("PUBCCRPTREQ")
    local dat,para = mqtt.pack(mqtt.PUBLISH,{qos=1,topic=entopic("devdata"),payload=protoair.pack(protoair.CCRPT,cctyp,ccnum,starttm,ringtm,totaltm)})
    if not snd(dat,{typ="MQTTPUBCCRPTREQ",val=para}) then sys.dispatch("SND_CCRPT_CNF",false) end
    return true
end

local cmds = {
	[protoair.SETPARA] = setpara,
	[protoair.SENDSMS] = sendsms,
	[protoair.DIAL] = dial,
	[protoair.QRYLOC] = qryloc,
	[protoair.RESET] = deveventrsp,
	[protoair.POWEROFF] = deveventrsp,	
	[protoair.RESTORE] = restore,
	[protoair.QRYPARA] = qrypara,
	[protoair.QRYRCD] = qryrcd,
}

--[[
函数名：publish
功能  ：解析mqtt发布的消息
参数  ：mqttpacket PUBLISH报文
返回值：无
]]
local function publish(mqttpacket)	
	local packet = protoair.unpack(mqttpacket.payload)
	if mqttpacket.qos == 1 then snd(mqtt.pack(mqtt.PUBACK,mqttpacket.seq)) end
	if packet and packet.id and cmds[packet.id] then
		packet.seq = mqttpacket.seq
		packet.topic = mqttpacket.topic
		cmds[packet.id](packet)
	end
end

--[[
函数名：pingrsp
功能  ：PING应答，
参数  ：mqttpacket PUBLISH报文
返回值：无
]]
local function pingrsp()
	sys.timer_stop(disconnect)
end

local mqttcmds = {
	[mqtt.CONNACK] = connack,
	[mqtt.SUBACK] = suback,
	[mqtt.PUBACK] = puback,
	[mqtt.PUBLISH] = publish,
	[mqtt.PINGRSP] = pingrsp,
}

--[[
函数名：mqttconncb
功能  ：连接回调函数
参数  ：v
返回值：无
]]
local function mqttconncb(v)
	--print("mqttconncb",common.binstohexs(v))
	mqttdup.ins("CONN",v)
end

--[[
函数名：mqttsubcb
功能  ：订阅回调函数
参数  ：v
返回值：无
]]
function mqttsubcb(v)
	mqttdup.ins("SUB",mqtt.pack(mqtt.SUBSCRIBE,v),v.seq)
end

--[[
函数名：mqttdupcb
功能  ：dup回调函数
参数  ：v
返回值：无
]]
local function mqttdupcb(v)
	mqttdup.rsm(v)
end

--[[
函数名：mqttdiscb
功能  ：mqtt断开回调函数
参数  ：无
返回值：无
]]
local function mqttdiscb()
	linkapp.sckdisc(SCK_IDX)
end

--[[
函数名：mqttpublocb
功能  ：上报位置回调函数
参数  ：v，r
返回值：无
]]
local function mqttpublocb(v,r)
	startloctimer()
	locshk = "SIL"
	if r then
		sys.dispatch("ITV_WAKE_SNDSUC")
		starthearttimer()	
		firstloc = true
		
		local function lbs1cb()
			manage.setlastlbs1(smatch(v.para.lbs1,"(%d+)@"),smatch(v.para.lbs1,"@(%d+)"),true)
		end
		
		local function lbs2cb()
			manage.setlastlbs2(v.para.lbs2,true)
		end
		
		local function gpscb()
			manage.setlastgps(smatch(v.para.gps,"([%.%d]+)@"),smatch(v.para.gps,"@([%.%d]+)"))
			manage.setlastlbs1(smatch(v.para.lbs1,"(%d+)@"),smatch(v.para.lbs1,"@(%d+)"))
			manage.setlastlbs2(v.para.lbs2)
		end
		
		local function gpslbscb()
			if v.para.gpsfix then manage.setlastgps(smatch(v.para.gps,"([%.%d]+)@"),smatch(v.para.gps,"@([%.%d]+)")) end
			manage.setlastlbs2(v.para.lbs2,not v.para.gpsfix)
		end
		
		local function gpslbswificb()
			if v.para.gpsfix then manage.setlastgps(smatch(v.para.gps,"([%.%d]+)@"),smatch(v.para.gps,"@([%.%d]+)")) end
			if v.para.wifi and #v.para.wifi > 0 then
				manage.setlastwifi(v.para.wifi)
				manage.setlastwifictl(v.para.wifi)
			end
			manage.setlastlbs2(v.para.lbs2,not (v.para.gpsfix or (v.para.wifi and #v.para.wifi > 0)))
		end
		
		local procer =
		{
			[protoair.LBS1] = lbs1cb,
			[protoair.LBS2] = lbs2cb,
			[protoair.LBS3] = lbs2cb,
			[protoair.GPS] = gpscb,
			[protoair.GPSLBS] = gpslbscb,
			[protoair.GPSLBS1] = gpslbscb,
			[protoair.GPSLBSWIFIEXT] = gpslbswificb,
		}
		if procer[v.typ] then procer[v.typ]() end
	end
end

--[[
函数名：mqttpubheartcb
功能  ：上报心跳回调函数
参数  ：v，r
返回值：无
]]
local function mqttpubheartcb(v,r)
	--[[if nvm.get("sleep") then
		sys.dispatch("ITV_SLEEP_REQ")
	else]]if nvm.get("gpsleep") then
		--sys.dispatch("ITV_GPSLEEP_REQ")
		end
	-- else
		if r then starthearttimer() end
	--end
	if r then sys.dispatch("ITV_WAKE_SNDSUC") end
end

--[[
函数名：mqttpubrptparacb
功能  ：上报参数回调函数
参数  ：v
返回值：无
]]
function mqttpubrptparacb(v)
	print("mqttpubrptparacb",v.usr)
	mqttdup.ins("PUBRPTPARA"..v.usr,mqtt.pack(mqtt.PUBLISH,v),v.seq)
end

--[[
函数名：mqttpubsetpararspcb
功能  ：设置参数回调函数
参数  ：v
返回值：无
]]
function mqttpubsetpararspcb(v)
	print("mqttpubsetpararspcb",v.usr)
	mqttdup.ins("PUBSETPARARSP"..v.usr,mqtt.pack(mqtt.PUBLISH,v),v.seq)
end

--[[
函数名：mqttpubdeveventrspcb
功能  ：事件应对回调函数
参数  ：v
返回值：无
]]
function mqttpubdeveventrspcb(v)
	print("mqttpubdeveventrspcb",v.usr)
	mqttdup.ins("PUBDEVEVENTRSP"..v.usr,mqtt.pack(mqtt.PUBLISH,v),v.seq)
end

--[[
函数名：mqttpubqryrcb
功能  ：拾音上报回调函数
参数  ：v
返回值：无
]]
function mqttpubqryrcb(v)
    print("mqttpubqryrcb",v.usr)
    mqttdup.ins("PUBQRYRCD&"..v.usr,mqtt.pack(mqtt.PUBLISH,v),v.seq)
end

--[[
函数名：mqttpubsmsrptreqcb
功能  ：短信上报回调函数
参数  ：v
返回值：无
]]
function mqttpubsmsrptreqcb(v)
    print("mqttpubsmsrptreqcb",v.usr)
    mqttdup.ins("PUBSMSRPTREQ",mqtt.pack(mqtt.PUBLISH,v),v.seq)
end

local sndcbs =
{
	MQTTCONN = mqttconncb,
	MQTTSUB = mqttsubcb,
	MQTTDUP = mqttdupcb,
	MQTTDISC = mqttdiscb,
	MQTTPUBLOC = mqttpublocb,
	MQTTPUBHEART = mqttpubheartcb,
	MQTTPUBRPTPARA = mqttpubrptparacb,
	MQTTPUBSETPARARSP = mqttpubsetpararspcb,
	MQTTPUBDEVEVENTRSP = mqttpubdeveventrspcb,
	MQTTPUBQRYRCD = mqttpubqryrcb,
	MQTTPUBSMSRPTREQ=mqttpubsmsrptreqcb,
}

--[[
函数名：enpwd
功能  ：密码处理
参数  ：s传入的密码
返回值：处理后的密码
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
函数名：ntfy
功能  ：对收到的CONNECT、STATE、DISCONNECT、SEND事件做相应处理
参数  ：idx,evt,result,item
返回值：无
]]
function ntfy(idx,evt,result,item)
	print("ntfy",evt,result)
	if evt == "CONNECT" then
		if result then
			if nvm.get("workmod") == "SMS" then
				linkapp.sckdisc(SCK_IDX)
			else
				sys.timer_stop(reconn)
				reconntimes = 0
				rests = ""
				
				local dat = mqtt.pack(mqtt.CONNECT,KEEP_ALIVE_TIME,tget["IMEI"](),tget["IMEI"](),enpwd(tget["IMEI"]()))
				mqttdup.rmv("CONN")
				if not snd(dat,"MQTTCONN") then mqttconncb(dat) end
				sys.dispatch("LINKAIR_CONNECT_SUC")
			end
		else
			sys.timer_start(reconn,5000)
		end
	elseif evt == "STATE" and result == "CLOSED" or evt == "DISCONNECT" then
		if nvm.get("gpsleep") then
			reconntimes,mqttconnfailcnt = 0,0
		else
			if mqttconnfailcnt <= 3 then sys.timer_start(reconn,5000) end
		end		
		sys.timer_stop(pingreq)
		mqttdup.rmvall()
		mqttconn = false
		sys.dispatch("LINKAIR_CONNECT_FAIL")
	elseif evt == "SEND" then
		if item.para then
			local typ = type(item.para) == "table" and item.para.typ or item.para
			local val = type(item.para) == "table" and item.para.val or item.data
			if sndcbs[typ] then sndcbs[typ](val,result)	end
		end
	end

	if smatch((type(result)=="string") and result or "","ERROR") then
		linkapp.sckdisc(SCK_IDX)
	end
end

--[[
函数名：rcv
功能  ：接收并解析接收到的报文
参数  ：id,data
返回值：无
]]
function rcv(id,data)
	print("rcv",common.binstohexs(data))
	sys.timer_start(pingreq,KEEP_ALIVE_TIME*1000/2)
	rests = rests..data

	local f,h,t = mqtt.iscomplete(rests)

	while f do
		data = ssub(rests,h,t)
		rests = ssub(rests,t+1,-1)
		local packet = mqtt.unpack(data)
		if packet and packet.typ and mqttcmds[packet.typ] then
			mqttcmds[packet.typ](packet)
			if packet.typ ~= mqtt.CONNACK and packet.typ ~= mqtt.SUBACK then
				checkdatactive()				
			end
		end
		f,h,t = mqtt.iscomplete(rests)
	end
end

--[[
函数名：mqttdupind
功能  ：重发数据
参数  ：s
返回值：无
]]
local function mqttdupind(s)
	if not snd(s,"MQTTDUP") then mqttdupcb(s) end
end

--[[
函数名：mqttdupfail
功能  ：消息重发失败
参数  ：t,s
返回值：无
]]
local function mqttdupfail(t,s)
    if smatch(t,"^PUBQRYRCD&") then
        local seq,cur = smatch(t,"PUBQRYRCD&(%d+)!(%d+)")
        sys.dispatch("SND_QRYRCD_CNF",false,seq,cur)
    end
end

--[[
函数名：shkind
功能  ：震动消息处理函数
参数  ：无
返回值：true
]]
local function shkind()
	print("shkind",locshk)
	local oldflg = locshk
	locshk = "SHK"
	if oldflg == "TMOUT" then
		preloc()
	end
	
	shkcnt = shkcnt + 1
	return true
end

--[[
函数名：chgind
功能  ：DEV_CHG_IND消息处理函数
参数  ：evt电池消息事件
返回值：true
]]
local function chgind(evt)
	if nvm.get("workmod") == "SMS" then return true end
	if evt ~= "CHARGER" then return true end
	chgalmrpt()
	if chg.getcharger() then
		chgalm = false
		stopalmtimer("CHG")
	elseif not sys.timer_is_active(almtimerfnc,"CHG") then
		startalmtimer("CHG",nvm.get("guard"))
	end
	return true
end

--[[
函数名：cconnect
功能  ：电话接通后需关掉datinactive定时器
参数  ：无
返回值：无
]]
local function cconnect()
	--if nvm.get("workmod") == "SMS" then return end
	sys.timer_stop(datinactive)
end

--[[
函数名：ccdisc
功能  ：电话挂断后重连后台
参数  ：无
返回值：无
]]
local function ccdisc()
	if nvm.get("workmod") == "SMS" then return end
	checkdatactive()
	reconntimes = 0
	connect(linkapp.NORMAL)
end

--[[
函数名：delaysmsmod
功能  ：延时短信工作模式的切换，工作模式中暂无短信工作模式，所以此函数暂时未用到
参数  ：无
返回值：无
]]
local function delaysmsmod()
	linkapp.sckclrsnding(SCK_IDX)
	link.shut()
	cconnect()
end

--[[
函数名：workmodind
功能  ：工作模式切换处理函数
参数  ：r,rpt
返回值：无
]]
local function workmodind(r,rpt)
	if nvm.get("workmod") == "SMS" then
		sys.timer_start(delaysmsmod,10000)
		--delaysmsmod()
	else
		sys.timer_stop(delaysmsmod)
		locshk,firstloc,firstgps = "SIL"
		chgalm,shkalm = true,true
		alminit()
		manage.resetlastloc()
		ccdisc()
	end
	if not rpt and r~="SVR" then
		nvm.set("workmodpend",true)
	end
	heart()
end

--[[
函数名：gpstaind
功能  ：gps事件处理函数
参数  ：evt gps事件
返回值：true
]]
local function gpstaind(evt)
	if nvm.get("workmod") == "SMS" then return end
	if evt == gps.GPS_LOCATION_SUC_EVT and manage.getlastyp() ~= "GPS" --[[and not firstgps]] then
		--if loc() then firstgps=true end
	end
	return true
end

--[[
函数名：parachangeloc
功能  ：位置上报频率改变或者定位模式改变，上报一次位置信息
参数  ：无
返回值：无
]]
local function parachangeloc()
	locshk = "SHK"
	preloc()
end

--[[
函数名：rptgpsleep
功能  ：gps关闭，上报心跳包
参数  ：无
返回值：无
]]
local function rptgpsleep()
	if nvm.get("gpsleep") then
		heart()
	end
end

--[[
函数名：stachange
功能  ：设备由运动切换到静止状态，上报心跳包
参数  ：无
返回值：true
]]
local function stachange()
	if manage.getmovsta()=="SIL" then
		heart()
	end
    return true
end

--[[
函数名：tparachangeind
功能  ：由非服务器方修改了参数，则将修改后的参数上报后台
参数  ：k,kk,v,r
返回值：true
]]
local function parachangeind(k,v,r)
	print("parachangeind",k,r)
	local rpt
	if r ~= "SVR" and nvm.get("workmod") ~= "SMS" then
		rpt = rptpara(k)
	end
	local procer =
	{
		rptfreq = parachangeloc,
		almfreq = alminit,
		guard = alminit,
		workmod = workmodind,
		fixmod = parachangeloc,
		heart = devinfo,
		--gpsleep = rptgpsleep,	
	}
	if procer[k] then procer[k](r,rpt) end
	return true
end

--[[
函数名：tparachangeind
功能  ：由非服务器方修改了参数，则将修改后的参数上报后台
参数  ：k,kk,v,r
返回值：true
]]
local function tparachangeind(k,kk,v,r)
	if r ~= "SVR" then
		rptpara(k)
	end
	return true
end

--[[
函数名：imeirdy
功能  ：IMEI读取成功，则向后台上报tpara表中的参数值
参数  ：无
返回值：无
]]
local function imeirdy()	
	rptpara()
end

--[[local function keyind()
	if nvm.get("workmod")~="LONGPS" then
		nvm.set("lastworkmod",nvm.get("workmod"),"KEY",false)
		nvm.set("workmod","LONGPS","KEY",false)
		nvm.flush()
	else
		nvm.set("workmod",nvm.get("lastworkmod"),"KEY")
	end
	return true
end]]

--关机原因值,0x00按键关机,0x01低电关机
local pwoffcause

--[[
函数名：getpwoffcause
功能  ：获取关机原因值
参数  ：无
返回值：关机原因值
]]
function getpwoffcause()
	return pwoffcause
end

--[[
函数名：delpwoffcause
功能  ：删除关机原因值
参数  ：无
返回值：无
]]
function delpwoffcause()
    pwoffcause=nil
end

--[[
函数名：rsp_pwoff
功能  ：应答关机请求，1.如果主人号码不为空，给主人号码发送关机成功短信 2，发送心跳 3执行关机操作
参数  ：tag 关机原因
返回值：无
]]
local function rsp_pwoff(tag)
	if nvm.get("adminum")~="" then
		smsapp.send(nvm.get("adminum"),"关机成功！")
	end
	if tag == "BAT_LOW" then
		pwoffcause=1
	end
	
	heart() 
    nvm.set("abnormal",false)
	sys.timer_start(rtos.poweroff,6000)
	return true
end

--keyrptpos true表示按键按下的时候需要上报位置信息，false或nil则上报心跳
--key_val_sta 标记按键的状态信息（最高位[7:7]表示按键状态，0：短按，1长按；[6:0]表示按键值，0开机键，1状态键）
local keyrptpos,key_val_sta = true
--[[
函数名：resetkeyrptposflg
功能  ：恢复按键按下是否需要上报位置信息的flag值
参数  ：无
返回值：无
]]
local function resetkeyrptposflg()
	keyrptpos = true
end

--[[
函数名：delkeyvalsta
功能  ：获取按键状态值
参数  ：无
返回值：按键状态值
]]
function getkeyvalsta()
	return key_val_sta
end

--[[
函数名：delkeyvalsta
功能  ：删除按键状态值
参数  ：无
返回值：无
]]
function delkeyvalsta()
	key_val_sta=nil
end

--[[
函数名：keyrpt
功能  ：上报位置，位置类型为按键上报
参数  ：无
返回值：true
]]
local function keyrpt() 
	locrpt("KEYRPT")
end

--[[
函数名：keyind
功能  ：不管是长按还是短按，一分钟内有N按键操作，上报一次位置信息，N-1次心跳
参数  ：k按键值
返回值：true
]]
local function keyind(k)
	print("keyind",k,keyrptpos,key_val_sta)
	if keyrptpos then
		keyrptpos=false
		sys.timer_start(resetkeyrptposflg,60000)
		if gps.isfix() then
		    keyrpt()
		else
		    sys.timer_start(keyrpt,8000)
		end
	else
		heart(true)
	end
	return true
end

--[[
函数名：shortkey
功能  ：短按按键消息处理函数
参数  ：k按键值
返回值：true
]]
local function shortkey(k)
    --key_val_sta最高位[7:7]表示按键状态，0：短按，1长按；[6:0]表示按键值，0开机键，1状态键
	key_val_sta=0x0
	keyind(k)
	return true
end

--[[
函数名：longkey
功能  ：长按按键消息处理函数
参数  ：k按键值
返回值：true
]]
local function longkey(k)
    --key_val_sta最高位[7:7]表示按键状态，0：短按，1长按；[6:0]表示按键值，0开机键，1状态键
	key_val_sta=0x80
	keyind(k)
	return true
end

local procer =
{
	MQTT_DUP_IND = mqttdupind,
	MQTT_DUP_FAIL = mqttdupfail,
	DEV_SHK_IND = shkind,
	DEV_CHG_IND = chgind,
	CCAPP_CONNECT = cconnect,
	CCAPP_DISCONNECT = ccdisc,
	[gps.GPS_STATE_IND] = gpstaind,
	PARA_CHANGED_IND = parachangeind,
	TPARA_CHANGED_IND = tparachangeind,
	IMEI_READY = imeirdy,
	--MMI_KEYPAD_FIVETAP_IND = keyind,
	MMI_KEYPAD_LONGPRESS_IND = longkey,
	MMI_KEYPAD_IND = shortkey,
	REQ_PWOFF = rsp_pwoff,
	SND_QRYRCD_REQ = sndqryrcdreq,
	SMS_RPT_REQ = smsrptreq,
	CCRPT_REQ = ccrptreq,
    STA_CHANGE = stachange,
}

--注册app消息处理函数
sys.regapp(procer)
--如果3倍KEEP_ALIVE_TIME+半分钟还没连上后台则重启设备
checkdatactive()
net.startquerytimer()
--连接后台
connect(linkapp.NORMAL)

