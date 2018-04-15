--[[
模块名称：AGPS，全称Assisted Global Positioning System，GPS辅助定位管理(仅适用于u-blox的GPS模块)
模块功能：连接AGPS后台，下载GPS星历数据，写入GPS模块，加速GPS定位
模块最后修改时间：2017.02.20
]]

--[[
连接上后台后，应用层协议：
1、发送AGPS到后台
2、后台回复AGPSUPDATE,total,last,sum1,sum2,sum3,......,sumn
   total：包的总个数
   last：最后一个包的字节数
   sum1：第一个包数据的校验和
   sum2：第二个包数据的校验和
   sum3：第三个包数据的校验和
   ......
   sumn：第n个包数据的校验和
3、发送Getidx
   idx是包的索引，范围是1---total
   例如：假设升级文件为4000字节，
   Get1
   Get2
   Get3
   Get4
4、后台回复每个包的内容
   第一个字节和第二个字节，为包的索引，大端
   其余数据为星历数据
]]

--定义模块,导入依赖库
local base = _G
local table = require"table"
local lpack = require"pack"
local rtos = require"rtos"
local sys = require"sys"
local string = require"string"
local link = require"link"
local misc = require"misc"
local net = require"net"
local gps = require"gps"
local bit = require"bit"
module(...,package.seeall)

--加载常用的全局函数至本地
local print,tonumber,fly = base.print,base.tonumber,base.fly
local sfind,slen,ssub,sbyte,sformat,smatch,sgsub,schar,srep = string.find,string.len,string.sub,string.byte,string.format,string.match,string.gsub,string.char,string.rep
local send,dispatch = link.send,sys.dispatch

--[[
lid：socket id
isfix：GPS是否定位成功
agpsop: 是否打开agps
]]
local lid,isfix,agpsop
--[[
ispt：是否开启AGPS功能
itv：连接AGPS后台间隔，单位秒，默认2小时，是指2小时连接一次AGPS后台，更新一次星历数据
PROT,SVR,PORT：AGPS后台传输层协议、地址、端口
WRITE_INTERVAL：每个星历数据包写入GPS模块的间隔，单位毫秒
]]
local ispt,itv,PROT,SVR,PORT,WRITE_INTERVAL = true,(2*3600),"UDP","bs.openluat.com",12412,100
--[[
mode：AGPS功能工作模式，有以下两种（默认为0）
  0：自动连接后台、下载星历数据、写入GPS模块
  1：需要连接后台时，产生内部消息AGPS_EVT，用户程序处理这个消息决定是否需要连接；下载星历数据，写入GPS模块后，结束后产生内部消息AGPS_EVT，通知用户下载结果和写入结果
pwrcb:开机回调函数
]]
local mode,pwrcb = 0
--[[
gpssupport：是否有GPS模块
eph：从AGPS后台下载的星历数据
]]
local gpssupport,eph = true,""
--[[
GET_TIMEOUT：GET命令等待时间，单位毫秒
ERROR_PACK_TIMEOUT：错误包(包ID或者长度不匹配) 在一段时间后进行重新获取
GET_RETRY_TIMES：GET命令超时或者错误包时，当前包允许重试的最大次数
PACKET_LEN：每包的最大数据长度，单位字节
RETRY_TIMES：连接后台，下载数据过程结束后，会断开连接；如果此次下载过程失败，则会重新连接后台，重新从头开始下载。这个变量指的是允许重新连接后台下载的最大次数
]]
local GET_TIMEOUT,ERROR_PACK_TIMEOUT,GET_RETRY_TIMES,PACKET_LEN,RETRY_TIMES = 10000,5000,3,1024,3
--[[
state：状态机状态
IDLE：空闲状态
CHECK：“查询服务器星历数据”状态
UPDATE：“下载星历数据中”状态
total：包的总个数，例如星历数据为10221字节，则total=(int)((10221+1021)/1022)=11;升级文件为10220字节，则total=(int)((10220+1021)/1022)=10
last：最后一个包的字节数，例如升级文件为10225字节，则last=10225%1022=5;升级文件为10220字节，则last=1022
checksum：每个包星历数据的校验和存储表
packid：当前包的索引
getretries：获取每个包已经重试的次数
retries：重新连接后台下载，已经重试的次数
reconnect：是否需要重连后台
]]
local state,total,last,checksum,packid,getretries,retries,reconnect = "IDLE",0,0,{},0,0,1,false

--[[
函数名：startupdatetimer
功能  ：开启“连接后台，更新星历数据”定时器
参数  ：无
返回值：无
]]
local function startupdatetimer()
	--支持GPS并且支持AGPS
	if gpssupport and ispt then
		sys.timer_start(connect,itv*1000)
	end
end

--[[
函数名：gpsstateind
功能  ：处理GPS模块的内部消息
参数  ：
		id：gps.GPS_STATE_IND，不用处理
		data：消息参数类型
返回值：true
]]
local function gpsstateind(id,data)
	--GPS定位成功
	if data == gps.GPS_LOCATION_SUC_EVT or data == gps.GPS_LOCATION_UNFILTER_SUC_EVT then
		sys.dispatch("AGPS_UPDATE_SUC")
		startupdatetimer()
		isfix = true
		setsucstr()
	--GPS定位失败或者GPS关闭
	elseif data == gps.GPS_LOCATION_FAIL_EVT or data == gps.GPS_CLOSE_EVT then
		isfix = false
	--没有GPS芯片
	elseif data == gps.GPS_NO_CHIP_EVT then
		gpssupport = false
	end
	return true
end

--[[
函数名：calsum
功能  ：计算校验和
参数  ：
		str：要计算校验和的数据
返回值：校验和
]]
local function calsum(str)
	local sum,i = 0
	for i=1,slen(str) do
		sum = sum + sbyte(str,i)
	end
	return sum
end

--[[
函数名：errpack
功能  ：错误包处理
参数  ：
		str：要计算校验和的数据
返回值：校验和
]]
local function errpack()
	print("errpack")
	upend(false)
end

--[[
函数名：retry
功能  ：重试动作
参数  ：
		para：如果为STOP，则停止重试；否则，执行重试
返回值：无
]]
function retry(para)
	if state ~= "UPDATE" and state ~= "CHECK" then
		return
	end

	if para == "STOP" then
		getretries = 0
		sys.timer_stop(errpack)
		sys.timer_stop(retry)
		return
	end

	if para == "ERROR_PACK" then
		sys.timer_start(errpack,ERROR_PACK_TIMEOUT)
		return
	end

	getretries = getretries + 1
	if getretries < GET_RETRY_TIMES then
		if state == "UPDATE" then
			-- 未达重试次数,继续尝试获取升级包
			reqget(packid)
		else
			reqcheck()
		end
	else
		-- 超过重试次数,升级失败
		upend(false)
	end
end

--[[
函数名：reqget
功能  ：发送“获取第index包的请求数据”到服务器
参数  ：
		index：包的索引，从1开始
返回值：无
]]
function reqget(idx)
	send(lid,sformat("Get%d",idx))
	sys.timer_start(retry,GET_TIMEOUT)
end

--[[
函数名：getpack
功能  ：解析从服务器收到的一包数据
参数  ：
		data：包内容
返回值：无
]]
local function getpack(data)
	-- 判断包长度是否正确
	local len = slen(data)
	if (packid < total and len ~= PACKET_LEN) or (packid >= total and len ~= (last+2)) then
		print("getpack:len not match",packid,len,last)
		retry("ERROR_PACK")
		return
	end

	-- 判断包序号是否正确
	local id = sbyte(data,1)*256 + sbyte(data,2)%256
	if id ~= packid then
		print("getpack:packid not match",id,packid)
		retry("ERROR_PACK")
		return
	end

	--判断校验和是否正确
	local sum = calsum(ssub(data,3,-1))
	if checksum[id] ~= sum then
		print("getpack:checksum not match",checksum[id],sum)
		retry("ERROR_PACK")
		return
	end

	-- 停止重试
	retry("STOP")

	-- 保存星历包
	eph = eph .. ssub(data,3,-1)

	-- 获取下一包数据
	if packid == total then
		sum = calsum(eph)
		if checksum[total+1] ~= sum then
			print("getpack:total checksum not match",checksum[total+1],sum)
			upend(false)
		else
			upend(true)
		end
	else
		packid = packid + 1
		reqget(packid)
	end
end

--[[
函数名：upbegin
功能  ：解析服务器下发的星历包信息
参数  ：
		data：星历包信息
返回值：无
]]
local function upbegin(data)
	--包的个数，最后一包的字节数
	local d1,d2,p1,p2 = sfind(data,"AGPSUPDATE,(%d+),(%d+)")
	local i
	if d1 and d2 and p1 and p2 then
		p1,p2 = tonumber(p1),tonumber(p2)
		total,last = p1,p2
		local tmpdata = data
		--每包星历数据的校验和
		for i=1,total+1 do
			if d2+2 > slen(tmpdata) then
				upend(false)
				return false
			end
			tmpdata = ssub(tmpdata,d2+2,-1)
			d1,d2,p1 = sfind(tmpdata,"(%d+)")
			if d1 == nil or d2 == nil or p1 == nil then
				upend(false)
				return false
			end
			checksum[i] = tonumber(p1)
		end

		getretries,state,packid,eph = 0,"UPDATE",1,""
		--请求第1包
		reqget(packid)
		return true
	end

	upend(false)
	return false
end

function writeapgs(str)
	print("writeapgs",str,slen(str))
	local A,tmp,s1,s2 = 65,0
	for i = 2,slen(str)-1 do
		tmp = bit.bxor(tmp,sbyte(str,i))
	end
	if bit.rshift(tmp,4) > 9 then
		s1 = schar(bit.rshift(tmp,4) - 10 + A)
	else
		s1 = bit.rshift(tmp,4) + '0'
	end

	if bit.band(tmp,0x0f) > 9 then
		s2 = schar(bit.band(tmp,0x0f) - 10 + A)
	else
		s2 = bit.band(tmp,0x0f) + '0'
	end
	str = str..s1..s2..'\13'..'\10'..'\0'
	print("writeapgs str",str,slen(str))
	gpscore.write(str)
end

local function agpswr()
	print("agpswr")
	local clkstr,s,i = os.date("*t")
	local clk = common.transftimezone(clkstr.year,clkstr.month,clkstr.day,clkstr.hour,clkstr.min,clkstr.sec,8,0)
	s = string.format("%0d,%02d,%02d,%02d,%02d,%02d",clk.year,clk.month,clk.day,clk.hour,clk.min,clk.sec)
	local str = getagpstr()
	if str then
		str = str..s..'*'
		writeapgs(str)
		gps.closegps("AGPS")
		sys.dispatch("AGPS_WRDATE_SUC")
	end
	return true
end

local function bcd(d,n)
	local l = slen(d or "")
	local num
	local t = {}

	for i=1,l,2 do
		num = tonumber(ssub(d,i,i+1),16)

		if i == l then
			num = 0xf0+num
		else
			num = (num%0x10)*0x10 + num/0x10
		end

		table.insert(t,num)
	end

	local s = string.char(base.unpack(t))

	l = slen(s)

	if l < n then
		s = s .. string.rep("\255",n-l)
	elseif l > n then
		s = ssub(s,1,n)
	end

	return s
end

local function encellinfo(s)
	local ret,t,mcc,mnc,lac,ci,rssi,k,v,m,n,cntrssi = "",{}
	print("syy encellinfo",s)
	for mcc,mnc,lac,ci,rssi in string.gmatch(s,"(%d+)%.(%d+)%.(%d+)%.(%d+)%.(%d+);") do
		mcc,mnc,lac,ci,rssi = tonumber(mcc),tonumber(mnc),tonumber(lac),tonumber(ci),(tonumber(rssi) > 31) and 31 or tonumber(rssi)
		local handle = nil
		for k,v in pairs(t) do
			print("syy v.lac",v.lac,lac,v.mcc,mcc,v.mnc,mnc,#v.rssici)
			if v.lac == lac and v.mcc == mcc and v.mnc == mnc then
				if #v.rssici < 8 then
					table.insert(v.rssici,{rssi=rssi,ci=ci})
				end
				--handle = true
				break
			end
		end
		print("syy handle",handle)
		if not handle then
			table.insert(t,{mcc=mcc,mnc=mnc,lac=lac,rssici={{rssi=rssi,ci=ci}}})
		end
	end
	for k,v in pairs(t) do
		ret = ret .. lpack.pack(">HHb",v.lac,v.mcc,v.mnc)
		for m,n in pairs(v.rssici) do
			cntrssi = bit.bor(bit.lshift(((m == 1) and (#v.rssici-1) or 0),5),n.rssi)
			ret = ret .. lpack.pack(">bH",cntrssi,n.ci)
		end
	end

	return #t,string.char(#t)..ret
end

--[[
函数名：reqcheck
功能  ：发送“请求星历信息”数据到服务器
参数  ：无
返回值：无
]]
function reqcheck()
	state = "CHECK"
	local num,sr = encellinfo(net.getcellinfoext())
	link.send(lid,lpack.pack("bAbAA",1,string.char(0),0,bcd(misc.getimei(),8),sr))
	sys.timer_start(retry,GET_TIMEOUT)
end

--[[
函数名：upend
功能  ：下载结束
参数  ：
		succ：结果，true为成功，其余为失败
返回值：无
]]
function upend(succ)
	state = "IDLE"
	-- 停止充实定时器
	sys.timer_stop(retry)
	sys.timer_stop(errpack)
	-- 断开链接
	link.close(lid)
	getretries = 0
	if succ then
		reconnect = false
		retries = 0
		--写星历信息到GPS芯片
		print("eph rcv",slen(eph))
		--startwrite()
		startupdatetimer()
		if mode==1 then dispatch("AGPS_EVT","END_IND",true) end
	else
		if retries >= RETRY_TIMES then
			reconnect = false
			retries = 0
			startupdatetimer()
			if mode==1 then dispatch("AGPS_EVT","END_IND",false) end
		else
			reconnect = true
			retries = retries + 1
		end
	end
end

local agpsstr

local function setagpstr(str)
	agpsstr = str
end

function getagpstr(str)
	return agpsstr
end

function setsucstr()
	local lng,lat = smatch(gps.getgpslocation(),"[EW]*,(%d+%.%d+),[NS]*,(%d+%.%d+)")
	print("setsucstr,lng",lng,lat)
	if lng and lat then
		local str = '$PMTK741,'..lat..','..lng..',0,'
		setagpstr(str)
	end
end

local function unbcd(d)
	local byte,v1,v2
	local t = {}

	for i=1,slen(d) do
		byte = sbyte(d,i)
		v1,v2 = bit.band(byte,0x0f),bit.band(bit.rshift(byte,4),0x0f)

		if v1 == 0x0f then break end
		table.insert(t,v1)

		if v2 == 0x0f then break end
		table.insert(t,v2)
	end

	return table.concat(t)
end

local function trans(lat,lng)
	local la,ln = lat,lng
	if slen(lat)>10 then
		la = ssub(lat,1,10)
	elseif slen(lat)<10 then
		la = lat..srep("0",10-slen(lat))
	end
	if slen(lng)>10 then
		ln = ssub(lng,1,10)
	elseif slen(lng)<10 then
		ln = lng..srep("0",10-slen(lng))
	end
	
	local la1,ln1 = sgsub(ssub(la,1,3),"0",""),sgsub(ssub(ln,1,3),"0","")
	
	return la1.."."..ssub(la,4,-1),ln1.."."..ssub(ln,4,-1)
end


--[[
函数名：rcv
功能  ：socket接收数据的处理函数
参数  ：
        id ：socket id，程序可以忽略不处理
        data：接收到的数据
返回值：无
]]
local function rcv(id,data)
	base.collectgarbage()
	--停止重试定时器
	sys.timer_stop(retry)
	print("syy rcv",slen(data),(slen(data)<270) and common.binstohexs(data) or "")
	if slen(data) >=11 then
		local lat,lng,latdm,lngdm = trans(unbcd(ssub(data,2,6)),unbcd(ssub(data,7,11)))
		print("syy rcv",lat,lng)
		if not lat or not lng then return end
		local str = '$PMTK741,'..lat..','..lng..',0,'
		print("syy rcv str",str)
		setagpstr(str)
		if gps.isopen() then
			agpswr()	
		elseif not agpsop then
			gps.opengps("AGPS")
			agpsop = true
		end
		upend(true)
		return		
	end		
	if isfix or not gpssupport then
		upend(true)
		return
	end
	if state == "CHECK" then
		--返回了星历包总信息
		if sfind(data,"AGPSUPDATE") == 1 then
			upbegin(data)
			return
		end
	elseif state == "UPDATE" then
		if data ~= "ERR" then
			getpack(data)
			return
		end
	end

	upend(false)
	return
end

--[[
函数名：nofity
功能  ：socket状态的处理函数
参数  ：
        id：socket id，程序可以忽略不处理
        evt：消息事件类型
		val： 消息事件参数
返回值：无
]]
local function nofity(id,evt,val)
	print("agps notify",lid,id,evt,val,reconnect)
	if id ~= lid then return end
	--如果定位成功或者不支持GPS模块
	if isfix or not gpssupport then
		upend(true)
		return
	end
	if evt == "CONNECT" then
		--连接成功
		if val == "CONNECT OK" then
			reqcheck()
		--连接失败
		else
			upend(false)
		end
	elseif evt == "CLOSE" and reconnect then
		--重连
		connect()
	elseif evt == "STATE" and val == "CLOSED" then
		upend(false)
	end
end

local function flycb()
	retries = RETRY_TIMES
	upend(false)
end

--[[
函数名：connectcb
功能  ：连接服务器
参数  ：无
返回值：无
]]
local function connectcb()
	lid = link.open(nofity,rcv,"agps")
	link.connect(lid,PROT,SVR,PORT)
end

--[[
函数名：connect
功能  ：连接服务器请求
参数  ：无
返回值：无
]]
function connect()
	if ispt then
		--自动模式
		if mode==0 then
			connectcb()
		--用户控制模式
		else
			dispatch("AGPS_EVT","BEGIN_IND",connectcb)
		end
	end
end

--[[
函数名：init
功能  ：设置连接服务器更新星历数据间隔和此模块工作模式
参数  ：
		inv：更新间隔，单位秒
		md：工作模式
返回值：无
]]
function init(inv,md)
	itv = inv or itv
	mode = md or 0
	startupdatetimer()
end

--[[
函数名：setspt
功能  ：设置是否开启AGPS功能
参数  ：
		spt：true为开启，false或者nil为关闭
返回值：无
]]
function setspt(spt)
	if spt ~= nil and ispt ~= spt then
		ispt = spt
		if spt then
			startupdatetimer()
		end
	end
end

--[[
函数名：load
功能  ：运行此功能模块
参数  ：无
返回值：无
]]
local function load(force)
	local pwrstat = pwrcb and pwrcb()
	if (gps.isagpspwronupd() or force) then
		connect()
	else
		startupdatetimer()
	end
end

function setpwrcb(cb)
	pwrcb = cb
	load(true)
end

local procer =
{
	AGPS_WRDATE = agpswr
}

--注册GPS消息处理函数
sys.regapp(gpsstateind,gps.GPS_STATE_IND)
sys.regapp(procer)
load()
if fly then fly.setcb(flycb) end
