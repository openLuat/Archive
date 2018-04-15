--[[
模块名称：协议模块
模块功能：初始化，程序运行框架、消息分发处理、定时器接口
模块最后修改时间：2017.02.09
]]

--定义模块,导入依赖库
require"logger"
local lpack = require"pack"
module(...,package.seeall)

--加载常用的全局函数至本地
local slen,sbyte,ssub,sgsub,schar,srep,smatch,sgmatch = string.len,string.byte,string.sub,string.gsub,string.char,string.rep,string.match,string.gmatch

--GPS GPS定位报文指令ID
--LBS1 _LBS定位报文1指令ID
--LBS2 _LBS定位报文2指令ID
--HEART 心跳报文指令ID
--LBS3 LBS定位报文3指令ID
--GPSLBS GPS&多LBS定位报文指令ID
--GPSLBS1 GPS&多LBS定位报文1指令ID
--GPSLBSWIFIEXT GPS&LBS &WIFI扩展位置报文指令ID
--GPSLBSWIFI GPS&LBS &WIFI定位报文指令ID
--RPTPARA 参数变化报文指令ID
--SETPARARSP 参数被动变化的应答报文指令ID
--DEVEVENTRSP 被动处理事件的应答报文指令ID
--GPSLBSWIFI1 GPS&LBS &WIFI定位报文1指令ID
--INFO 终端信息报文指令ID
GPS,LBS1,LBS2,HEART,LBS3,GPSLBS,GPSLBS1,GPSLBSWIFIEXT,GPSLBSWIFI,RPTPARA,SETPARARSP,DEVEVENTRSP,GPSLBSWIFI1,INFO = 1,2,3,4,5,6,7,8,9,10,11,12,13,14
--RPTFREQ 位置报文上报间隔
--ALMFREQ 报警间隔
--GUARDON 设防状态
--GUARDOFF 撤防状态
--RELNUM 亲情号码
--CALLVOL 通话音量
--CALLRINGVOL 亲情号码来电铃声音量
--CALLRINGMUTEON 来电静音开启
--CALLRINGMUTEOFF 来电静音关闭
RPTFREQ,ALMFREQ,GUARDON,GUARDOFF,RELNUM,CALLVOL,CALLRINGVOL,CALLRINGMUTEON,CALLRINGMUTEOFF = 3,4,5,6,15,16,17,18,19
--SILTIME 上课静默时间段
--FIXTM 定时定位时间点
--WHITENUM 呼入白名单
--FIXMOD 定位模式
--SMSFORBIDON 短信屏蔽开启
--SMSFORBIDOFF 短信屏蔽关闭
--CALLRINGID 亲情号码来电铃声类型
--SOSIND SOS报警
--ENTERTAIN 娱乐时间设置
--ALARM 闹钟设置
SILTIME,FIXTM,WHITENUM,FIXMOD,SMSFORBIDON,SMSFORBIDOFF,CALLRINGID,SOSIND,ENTERTAIN,ALARM = 20,21,22,23,24,25,26,27,28,29
--SETPARA参数变化报文指令ID
--SENDSMS发送短信报文指令ID
--DIAL拨打电话报文指令ID
--QRYLOC主动定位报文指令ID
--RESET重启报文指令ID
--MONIT监听报文指令ID
--POWEROFF关机报文指令ID
--RESTORE恢复出厂设置报文指令ID
--PROMPT提示语报文指令ID
--QRYPARA查询参数报文指令ID
--QRYRCD拾音控制报文指令ID
SETPARA,SENDSMS,DIAL,QRYLOC,RESET,MONIT,POWEROFF,RESTORE,PROMPT,QRYPARA,QRYRCD = 10,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x50,0x51
--QRYRCDREQ拾音报文指令ID
--SMSRPT 短信报文指令ID
--CCRPT 电话报文指令ID
QRYRCDREQ,SMSRPT,CCRPT=0x17,0x18,0x19
--版本号对应的参数ID
VER = 0
--FIXTIMEPOS,QRYPOS,KEYPOS为GPS&LBS &WIFI扩展位置报文中位置类型
--FIXTIMEPOS设备定时定位上报
--QRYPOS设备对后台主动定位请求的应答上报
--KEYPOS设备按键上报
FIXTIMEPOS,QRYPOS,KEYPOS = 0,1,6

local PROTOVERSION = 0
local serial = 0
local imei -- 命令头包含IMEI,在第一次获取以后做缓存
local log = logger.new("PROTOAIR","BIN2")
local get

local function print(...)
	_G.print("protoair",...)
end

--[[
函数名：bcd
功能  ：将普通的字符串转成BCD码
参数  ：d,n
返回值：BCD码
]]
function bcd(d,n)
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

	local s = schar(_G.unpack(t))

	l = slen(s)

	if l < n then
		s = s .. srep("\255",n-l)
	elseif l > n then
		s = ssub(s,1,n)
	end

	return s
end

--[[
函数名：unbcd
功能  ：将BCD码转成普通的字符串
参数  ：d
返回值：字符串
]]
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

--[[
函数名：enlnla
功能  ：经纬度的封包处理
参数  ：v,true定位成功，否则定位失败
        s经度或者纬度值
返回值：字符串
]]
local function enlnla(v,s)
	if not v then return common.hexstobins("FFFFFFFFFF") end
	
	local v1,v2 = smatch(s,"(%d+)%.(%d+)")
	if not v1 or not v2 then return common.hexstobins("FFFFFFFFFF") end

	if slen(v1) < 3 then v1 = srep("0",3-slen(v1)) .. v1 end

	return bcd(v1..v2,5)
end

--[[
函数名：enstat
功能  ：基本状态信息封包处理
参数  ：无
返回值：基本状态信息封包字符串
]]
local function enstat()
	local stat = get("STATUS")
	local rssi = get("RSSI")
	local gpstat = get("GPSTAT")
	local satenum = gpstat.satenum
	local chgstat = chg.getstate()
	local pwoffcause = linkair.getpwoffcause()
	local key_val_sta = linkair.getkeyvalsta()
	print("enstat",key_val_sta)

	-- 状态字节1
	local n1 = stat.shake + stat.charger*2 + stat.acc*4 + stat.gps*8 + stat.sleep*16
	-- 状态字节2
	rssi = rssi > 31 and 31 or rssi
	satenum = satenum > 7 and 7 or satenum
	local n2 = rssi + satenum*32

	local base = lpack.pack(">bbH",n1,n2,stat.volt)
	local extend
    --静止运行状态，0x00静止状态,0x01运行状态
	if get("MOVSTA")=="SIL" then
		extend = lpack.pack(">bHb",5,1,0)
	else
		extend = lpack.pack(">bHb",5,1,1)   
	end

    --充电状态，0x00未充电（未连接充电器）,0x01充电中（连接充电器），0x02充电完成（连接充电器）
	if chgstat == 0 then
		extend = extend..lpack.pack(">bHb",6,1,0)
	elseif chgstat == 1 then
		extend = extend..lpack.pack(">bHb",6,1,1)
	elseif chgstat == 2 then
		extend = extend..lpack.pack(">bHb",6,1,2)	
	end
	
    --关机原因，0x00按键关机,0x01低电关机
	if pwoffcause and pwoffcause <= 1 then
        extend = extend..lpack.pack(">bHb",7,1,pwoffcause)
        linkair.delpwoffcause()
    end
	
    --按键，最高位[7:7]表示按键状态，0：短按，1长按；[6:0]表示按键值，0开机键，1状态键
	if key_val_sta ~=nil then
		extend = extend..lpack.pack(">bHb",8,1,key_val_sta)
		linkair.delkeyvalsta()
	end
	
	return base..extend
end

--[[
函数名：encellinfo
功能  ：基站定位信息封包处理
参数  ：无
返回值：基基站定位信息封包字符串
]]
local function encellinfo()
	local info,ret,t,lac,ci,rssi,k,v,m,n,cntrssi = get("CELLINFO"),"",{}
	print("encellinfo",info)
	for lac,ci,rssi in sgmatch(info,"(%d+)%.(%d+)%.(%d+);") do
		lac,ci,rssi = tonumber(lac),tonumber(ci),(tonumber(rssi) > 31) and 31 or tonumber(rssi)
		local handle = nil
		for k,v in pairs(t) do
			if v.lac == lac then
				if #v.rssici < 8 then
					table.insert(v.rssici,{rssi=rssi,ci=ci})
				end
				handle = true
				break
			end
		end
		if not handle then
			table.insert(t,{lac=lac,rssici={{rssi=rssi,ci=ci}}})
		end
	end
	for k,v in pairs(t) do
		ret = ret .. lpack.pack(">H",v.lac)
		for m,n in pairs(v.rssici) do
			cntrssi = bit.bor(bit.lshift(((m == 1) and (#v.rssici-1) or 0),5),n.rssi)
			ret = ret .. lpack.pack(">bH",cntrssi,n.ci)
		end
	end

	return schar(#t)..ret
end

--[[
函数名：encellinfoext
功能  ：扩展基站定位信息封包处理
参数  ：无
返回值：扩展基基站定位信息封包字符串
]]
local function encellinfoext()
	local info,ret,t,mcc,mnc,lac,ci,rssi,k,v,m,n,cntrssi = get("CELLINFOEXT"),"",{}
	print("encellinfoext",info)
	for mcc,mnc,lac,ci,rssi in sgmatch(info,"(%d+)%.(%d+)%.(%d+)%.(%d+)%.(%d+);") do
		mcc,mnc,lac,ci,rssi = tonumber(mcc),tonumber(mnc),tonumber(lac),tonumber(ci),(tonumber(rssi) > 31) and 31 or tonumber(rssi)
		local handle = nil
		for k,v in pairs(t) do
			if v.lac == lac and v.mcc == mcc and v.mnc == mnc then
				if #v.rssici < 8 then
					table.insert(v.rssici,{rssi=rssi,ci=ci})
				end
				handle = true
				break
			end
		end
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

	return schar(#t)..ret
end

--[[
函数名：enwifi
功能  ：wifi热点信息封包处理
参数  ：p，WIFI热点信息
返回值：wifi热点信息封包字符串
]]
local function enwifi(p)
	local t,ret,i,mac,rssi = p or {},""
	for i=1,#t do
		mac,rssi = common.hexstobins(sgsub(t[i].mac,":","")),schar(255+tonumber(t[i].rssi))
		if mac and rssi then
			ret = ret..mac..rssi
		end
	end
	return schar(#t)..ret
end

--[[
函数名：pack
功能  ：根据协议文档将命令封包处理
参数  ：id，命令id
返回值：报文字符串
]]
function pack(id,...)
	if not imei then imei = bcd(get("IMEI"),8) end

	local head = schar(id)

    --[[
    函数名：gps
    功能  ：gps定位报文封包处理
    参数  ：无
    返回值：字符串
    ]]
	local function gps()
		local t = get("GPS")
		lng = enlnla(t.fix,t.lng)
		lat = enlnla(t.fix,t.lat)

		return lpack.pack(">AAHbA",lng,lat,t.cog,t.spd,enstat())
	end

    --[[
    函数名：lbs1
    功能  ：LBS定位报文1封包处理
    参数  ：无
    返回值：字符串
    ]]
	local function lbs1()
		local ci,lac = get("CELLID"),get("LAC")
		return lpack.pack(">HbIHA",get("MCC"),get("MNC"),ci,lac,enstat())
	end
    
    --[[
    函数名：lbs3
    功能  ：LBS定位报文3封包处理
    参数  ：无
    返回值：字符串
    ]]
	local function lbs3()
		return lpack.pack(">AbA",encellinfoext(),get("TA"),enstat())
	end
	
    --[[
    函数名：lbs2
    功能  ：LBS定位报文2封包处理
    参数  ：无
    返回值：字符串
    ]]
	local function lbs2()
		return lpack.pack(">HbAbA",get("MCC"),get("MNC"),encellinfo(),get("TA"),enstat())
	end
	
    --[[
    函数名：gpslbs
    功能  ：GPS&多LBS定位报文封包处理
    参数  ：无
    返回值：字符串
    ]]
	local function gpslbs()
		local t = get("GPS")
		lng = enlnla(t.fix,t.lng)
		lat = enlnla(t.fix,t.lat)
		return lpack.pack(">AAHbHbAbA",lng,lat,t.cog,t.spd,get("MCC"),get("MNC"),encellinfo(),get("TA"),enstat())
	end
	
    --[[
    函数名：gpslbs1
    功能  ：GPS&多LBS定位报文1封包处理
    参数  ：无
    返回值：字符串
    ]]
	local function gpslbs1()
		local t = get("GPS")
		lng = enlnla(t.fix,t.lng)
		lat = enlnla(t.fix,t.lat)
		return lpack.pack(">AAHbAbA",lng,lat,t.cog,t.spd,encellinfoext(),get("TA"),enstat())
	end

	--[[
    函数名：gpslbswifi
    功能  ：GPS&LBS &WIFI定位报文封包处理
    参数  ：无
    返回值：字符串
    ]]
	local function gpslbswifi(p)
		local t = get("GPS")
		lng = enlnla(t.fix,t.lng)
		lat = enlnla(t.fix,t.lat)
		return lpack.pack(">AAHbHbAbAA",lng,lat,t.cog,t.spd,get("MCC"),get("MNC"),encellinfo(),get("TA"),enwifi(p),enstat())
	end

	--[[
    函数名：gpslbswifi1
    功能  ：GPS&LBS &WIFI定位报文1封包处理
    参数  ：无
    返回值：字符串
    ]]
	local function gpslbswifi1(p)
		local t = get("GPS")
		lng = enlnla(t.fix,t.lng)
		lat = enlnla(t.fix,t.lat)
		return lpack.pack(">AAHbAbAA",lng,lat,t.cog,t.spd,encellinfoext(),get("TA"),enwifi(p),enstat())
	end

	--[[
    函数名：gpslbswifiext
    功能  ：GPS&LBS &WIFI扩展位置报文封包处理
    参数  ：无
    返回值：字符串
    ]]
	local function gpslbswifiext(w,typ)
		local t = get("GPS")
		lng = enlnla(t.fix,t.lng)
		lat = enlnla(t.fix,t.lat)
		return lpack.pack(">bAAHbAbAA",typ,lng,lat,t.cog,t.spd,encellinfoext(),get("TA"),enwifi(w),enstat())
	end

    --[[
    函数名：heart
    功能  ：心跳报文封包处理
    参数  ：无
    返回值：字符串
    ]]
	local function heart()
		return lpack.pack("A",enstat())
	end	
	
    --[[
    函数名：rptpara
    功能  ：参数变化报文封包处理
    参数  ：dat参数
    返回值：字符串
    ]]
	local function rptpara(dat)
		return dat or ""
	end
	
    --[[
    函数名：rsp
    功能  ：被动处理事件的应答报文封包处理
    参数  ：typ,应答事件类型
            result，应答结果
    返回值：字符串
    ]]
	local function rsp(typ,result)
		return lpack.pack("bA",typ,result)
	end

    --[[
    函数名：info
    功能  ：终端信息报文封包处理
    参数  ：无
    返回值：字符串
    ]]
	local function info()
		return lpack.pack(">bHHbHHbHAbHAbHA",0,2,get("PROJECTID"),1,2,get("HEART"),2,2,bcd(sgsub(get("VERSION"),"%.",""),2),4,slen(get("ICCID")),get("ICCID"),0x0D,slen(get("IMSI")),get("IMSI"))
	end
	
    --[[
    函数名：qryrcdreq
    功能  ：拾音报文封包处理
    参数  ：s报文序列号,w发送方式,t报文总条数,
            c当前报文索引,typ音频文件类型,tmlen音频时长,d当前报文内容
    返回值：字符串
    ]]
	local function qryrcdreq(s,w,t,c,typ,tmlen,d)
		return lpack.pack(">bbbbbb",s,w,t,c,typ,tmlen)..d
	end
	
    --[[
    函数名：smsrptreq
    功能  ：短信报文封包处理
    参数  ：t收到短信的时间,
            num,号码
            d，短信内容
    返回值：字符串
    ]]
	local function smsrptreq(t,num,d)
		local len,bcdnum,year,mon,day,h,m,s,gggg
		local d1,d2 = string.find(num,"^([%+]*86)")
		if d1 and d2 then num = string.sub(num,d2+1,-1) end
		len= (slen(num)+1)/2   
		bcdnum = bcd(num,len)
		year,mon,day,h,m,s=smatch(t,"(%d+)/(%d+)/(%d+),(%d+):(%d+):(%d+)")
		year,mon,day,h,m,s = tonumber(year),tonumber(mon),tonumber(day),tonumber(h),tonumber(m),tonumber(s)

		return lpack.pack(">bbbbbbbAA",year,mon,day,h,m,s,len,bcdnum,common.hexstobins(d))
	end
	
    --[[
    函数名：ccrptreq
    功能  ：电话报文封包处理
    参数  ：cctyp电话类型,0：来电，1：去电
            num,号码
            starttm，来电或去电时间
            ringtm,响铃时间
            totaltm，通话总时间
    返回值：字符串
    ]]
	local function ccrptreq(cctyp,num,starttm,ringtm,totaltm)
        local len,bcdnum,year,mon,day,h,m,s,gggg
        local d1,d2 = string.find(num,"^([%+]*86)")
        if d1 and d2 then num = string.sub(num,d2+1,-1) end
        len= (slen(num)+1)/2   
        bcdnum = bcd(num,len)
        year,mon,day = tonumber(ssub(starttm,1,2)),tonumber(ssub(starttm,3,4)),tonumber(ssub(starttm,5,6))
        h,m,s=tonumber(ssub(starttm,7,8)),tonumber(ssub(starttm,9,10)),tonumber(ssub(starttm,11,12))
        print("ccrptreq",cctyp,len,bcdnum,year,mon,day,h,m,s,ringtm,totaltm)   
        return lpack.pack(">bbAbbbbbbbI",cctyp,len,bcdnum,year,mon,day,h,m,s,ringtm,totaltm)
    end
	
	local procer = {
		[GPS] = gps,
		[LBS1] = lbs1,
		[LBS2] = lbs2,
		[LBS3] = lbs3,
		[GPSLBS] = gpslbs,
		[GPSLBS1] = gpslbs1,
		[GPSLBSWIFIEXT] = gpslbswifiext,
		[GPSLBSWIFI] = gpslbswifi,
		[GPSLBSWIFI1] = gpslbswifi1,
		[HEART] = heart,
		[RPTPARA] = rptpara,
		[SETPARARSP] = rsp,
		[DEVEVENTRSP] = rsp,
		[INFO] = info,
		[QRYRCDREQ] = qryrcdreq,
		[SMSRPT] = smsrptreq,
		[CCRPT] = ccrptreq,
	}	

	local s=lpack.pack("AA",head,procer[id](...))
	print("pack",id,(slen(s) > 200) and common.binstohexs(ssub(s,1,200)) or common.binstohexs(s))
	return s
end

--[[
函数名：unpack
功能  ：解包处理
参数  ：s，收到的报文字符串
返回值：字符串
]]
function unpack(s)
	local packet = {}	
	
    --[[
    函数名：setpara
    功能  ：参数变化报文解包处理
    参数  ：d，收到的报文字符串
    返回值：字符串
    ]]
	local function setpara(d)
		if slen(d) > 0 then			
			
			local function unsignshort(m)
				if slen(m) ~= 2 then return end
				_,packet.val = lpack.unpack(m,">H")
				return true
			end
			
			local function empty(m)
				return m==""
			end
			
			local function numlist(m)
				if m == "" then return end
				packet.val = {}
				local i = 1
				while i < slen(m) do
					if i+1 > slen(m) then return end
					if i+1+sbyte(m,i+1) > slen(m) then return end
					packet.val[sbyte(m,i)] = (sbyte(m,i+1)==0 and "" or unbcd(ssub(m,i+2,i+1+sbyte(m,i+1))))					
					i = i+2+sbyte(m,i+1)
				end
				return true
			end
			
			local function unsignchar(m)
				if slen(m) ~= 1 then return end
				packet.val = sbyte(m)
				return true
			end
			
			local function listunsignchar(m)
				if m == "" then return end
				packet.val = {}
				local i = 1
				while i < slen(m) do
					if i+1 > slen(m) then return end					
					packet.val[sbyte(m,i)] = sbyte(m,i+1)
					i = i+2					
				end
				return true
			end
			
			local function timesect(m)
				if m == "" then return end
				packet.val = {}
				local i,tmp,j = 1
				while i < slen(m) do
					if i+5 > slen(m) then return end
					local flg = sbyte(m,i+1)
					tmp = ""
					for j=1,7 do
						tmp = tmp..(bit.band(flg,2^j)==0 and "0" or "1")
					end
					tmp = tmp.."!"..string.format("%02d%02d%02d%02d",sbyte(m,i+2),sbyte(m,i+3),sbyte(m,i+4),sbyte(m,i+5))
					packet.val[sbyte(m,i)] = tmp
					i = i+6					
				end
				return true
			end
			
			local function alarm(m)
				if m == "" then return end
				packet.val = {}
				local i,tmp,j = 1
				while i < slen(m) do
					if i+3 > slen(m) then return end
					local flg = sbyte(m,i+1)
					tmp = ""
					for j=0,7 do
						tmp = tmp..(bit.band(flg,2^j)==0 and "0" or "1")
					end
					tmp = tmp.."!"..string.format("%02d%02d",sbyte(m,i+2),sbyte(m,i+3))
					packet.val[sbyte(m,i)] = tmp
					i = i+4
				end
				return true
			end
			
			local proc =
			{
				[RPTFREQ] = unsignshort,
				[ALMFREQ] = unsignshort,
				[GUARDON] = empty,
				[GUARDOFF] = empty,
				[RELNUM] = numlist,
				[CALLVOL] = unsignchar,
				[CALLRINGVOL] = listunsignchar,
				[CALLRINGMUTEON] = empty,
				[CALLRINGMUTEOFF] = empty,
				[SILTIME] = timesect,
				[FIXTM] = timesect,
				[WHITENUM] = numlist,
				[FIXMOD] = unsignchar,
				[SMSFORBIDON] = empty,
				[SMSFORBIDOFF] = empty,
				[CALLRINGID] = listunsignchar,
				[ENTERTAIN] = timesect,
				[ALARM] = alarm,
			}
			packet.cmd = sbyte(d)
			if not proc[sbyte(d)] then print("protoair.unpack:unknwon setpara",sbyte(d)) return end			
			return proc[sbyte(d)](ssub(d,2,-1)) and packet or nil
		end
	end
	
    --[[
    函数名：sendsms
    功能  ：发送短信报文解包处理
    参数  ：d，收到的报文字符串
    返回值：解包成功true，否则nil
    ]]
	local function sendsms(d)
		if d == "" then return end

		local numcnt,i = sbyte(d)
		if numcnt*6+1 >= slen(d) then return end
		packet.num = {}
		for i=1,numcnt do
			local n = unbcd(ssub(d,2+(i-1)*6,7+(i-1)*6))
			if n and slen(n) > 0 then
				table.insert(packet.num,n)
			end
		end

		local t = {"7BIT","UCS2"}
		local typ = sbyte(d,numcnt*6+2)+1
		if not t[typ] then return end
		packet.coding = t[typ]
		packet.data = ssub(d,numcnt*6+3,-1)
		if not packet.data or slen(packet.data) <= 0 then return end

		return true
	end

    --[[
    函数名：dial
    功能  ：拨打电话报文解包处理
    参数  ：d，收到的报文字符串
    返回值：解包成功true，否则nil
    ]]
	local function dial(d)
		if d == "" then return end

		local numcnt,i = sbyte(d)
		if numcnt*6 >= slen(d) then return end
		packet.num = {}
		for i=1,numcnt do
			local n = unbcd(ssub(d,2+(i-1)*6,7+(i-1)*6))
			if n and slen(n) > 0 then
				table.insert(packet.num,n)
			end
		end
		return true
	end
	
    --[[
    函数名：empty
    功能  ：报文内容为空的报文处理
    参数  ：d，收到的报文字符串
    返回值：解包成功true，否则nil
    ]]
	local function empty(d)
		return d==""
	end
	
    --[[
    函数名：prompt
    功能  ：提示语报文解包处理
    参数  ：d，收到的报文字符串
    返回值：解包成功true，否则nil
    ]]
	local function prompt(d)
		if d == "" then return end
		packet.data = d
		return true
	end
	
    --[[
    函数名：qrypara
    功能  ：查询参数报文解包处理
    参数  ：d，收到的报文字符串
    返回值：解包成功true，否则nil
    ]]
	local function qrypara(d)
		if d == "" then return end
		packet.val = sbyte(d)
		return true
	end
	
    --[[
    函数名：qryrcd
    功能  ：拾音控制报文解包处理
    参数  ：d，收到的报文字符串
    返回值：解包成功true，否则nil
    ]]
	local function qryrcd(d)
		if slen(d) ~= 2 then return end
		packet.val = sbyte(ssub(d,1,1))
		packet.typ = sbyte(ssub(d,2,-1))
		return true
	end
	
	local procer = {
		[SETPARA] = setpara,
		[SENDSMS] = sendsms,
		[DIAL] = dial,
		[QRYLOC] = empty,
		[RESET] = empty,
		[MONIT] = dial,
		[POWEROFF] = empty,
		[RESTORE] = empty,
		[PROMPT] = prompt,
		[QRYPARA] = qrypara,
		[QRYRCD] = qryrcd,
	}	
	local id = sbyte(s,1)
	if not procer[id] then print("protoair.unpack:unknwon id",id) return end
	packet.id = id
	print("unpack",id,common.binstohexs(s))
	return procer[id](ssub(s,2,-1)) and packet or nil
end

function reget(id)
	get = id
end
