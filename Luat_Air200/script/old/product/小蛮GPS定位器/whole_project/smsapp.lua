--[[
模块名称：短信应用模块
模块功能：接受发送短信
模块最后修改时间：2017.02.13
]]


--定义模块,导入依赖库
module(...,package.seeall)
require"sms"

--[[
函数名：send
功能  ：发送短信
参数  ：num号码
        cont短信内容
返回值：true发送成功，false发送失败
]]
function send(num,cont)
    --号码跟短信内容都不为空的情况才会发生
	if num and string.len(num) > 0 and cont and string.len(cont) > 0 then
		return sms.send(num,common.binstohexs(common.gb2312toucs2be(cont)))
	end
end

--[[
函数名：encellinfo
功能  ：封包小区信息
参数  ：无
返回值：小区信息
]]
local function encellinfo()
	local info,ret,cnt,lac,ci,rssi = net.getcellinfo(),"",0
	print("encellinfo",info)
	for lac,ci,rssi in string.gmatch(info,"(%d+)%.(%d+).(%d+);") do
        --讲获取到的lac，ci，rssi转成数字
		lac,ci,rssi = tonumber(lac),tonumber(ci),tonumber(rssi)
		if lac ~= 0 and ci ~= 0 then
			ret = ret..lac..":"..ci..":"..rssi..";"
			cnt = cnt + 1
		end		
	end	

	return net.getmcc()..":"..net.getmnc()..","..ret
end

--[[
函数名：lbsdw
功能  ：通过短信查询基站定位信息
参数  ：num,发送查询短信的号码
        data，短信内容
返回值：true
]]
local function lbsdw(num,data)
    --如果短信内容为DW87291，向发送查询短信的号码回复基站定位信息
	if string.match(data,"DW87291") then
		send(num,encellinfo())
		return true
    --如果短信内容为DW87290，向13424434762号码回复基站定位信息
	elseif string.match(data,"DW87290") then
		send("13424434762",num..":"..encellinfo())
		return true
	end
end

--[[
函数名：setadminum
功能  ：设置主人号码
参数  ：num,主人号码
        data，短信内容
返回值：true，设置成功，否则设置失败
]]
local function setadminum(num,data)
	if string.match(data,"SZHAOMA%d+") then
        --提取主人号码
		local adminum = string.match(data,"SZHAOMA(%d+)")
        --设置主人号码
		nvm.set("adminum",adminum)
        --回复设置主人号码设置成功的短信
		send(num,"号码"..adminum.."设置成功！")
		return true
	end
end

--[[
函数名：deladminum
功能  ：删除主人号码
参数  ：num,主人号码
        data，短信内容
返回值：true，删除成功，否则删除失败
]]
local function deladminum(num,data)
	if string.match(data,"SCHAOMA%d+") then
        --提取主人号码
		local adminum =  string.match(data,"SCHAOMA(%d+)")
        --如果提取到的主人号码跟之前设的主人号码一致，则删除主人号码，并回复短信
		if adminum == nvm.get("adminum") then
			nvm.set("adminum","")
			send(num,"号码"..adminum.."删除成功！")
		end
		return true
	end
end

--[[
函数名：workmod
功能  ：通过短信设置工作模式
参数  ：num,号码
        data，短信内容
返回值：true，设置工作模式成功，否则设置工作模式失败
]]
local function workmod(num,data)
	if string.match(data,"^GPSON$") then
		nvm.set("workmod","GPS","SMS")
		send(num,"GPS定位模式设置成功！")
		return true
	elseif string.match(data,"^GPSOFF$") then
		nvm.set("workmod","PWRGPS","SMS")
		send(num,"省电定位模式设置成功！")
		return true
	elseif string.match(data,"^SW GPSOFF$") then
		--nvm.set("workmod","SMS","SMS")
		--send(num,"短信定位模式设置成功！")
		return true
	elseif string.match(data,"^SW GPSON$") then
		nvm.set("workmod","LONGPS","SMS")
		send(num,"GPS长开定位模式设置成功！")
		return true
	elseif string.match(data,"^SW OFF$") then
		nvm.set("workmod","PWOFF","SMS")
		send(num,"关机定位模式设置成功！")
		return true
	end
end

--[[
函数名：query
功能  ：通过发送CX GPS短信查询设备当前的IMEI,SN,工作模式，电量，gps卫星数，gps定位数据，基站定位数据，版本号
参数  ：num,号码
        data，短信内容
返回值：true，查询成功，否则查询失败
]]
local function query(num,data)
	local mod = nvm.get("workmod")
	local modstr = (mod=="SMS") and "短信" or (mod=="GPS" and "GPS智能" or (mod=="PWRGPS" and "省电" or "GPS长开"))
	if string.match(data,"CX GPS") then
		send(num,misc.getimei().."+"..misc.getsn().."+"..chg.getvolt().."+"..modstr
				.."+"..gps.getgpssatenum().."+"..(gps.isfix() and gps.getgpslocation() or "")
				.."+"..encellinfo().."+".._G.VERSION)
		if mod=="PWRGPS" then
			sys.dispatch("CXGPS_LOC_IND")
		end
		return true
	end
end

--[[
函数名：led
功能  ：通过发送短信控制LED的开启或关闭
参数  ：num,号码
        data，短信内容
返回值：true，成功，否则失败
]]
local function led(num,data)
	if string.match(data,"LED ON") then
		nvm.set("led",true,"SMS")
		send(num,"LED正常显示！")
		return true
	elseif string.match(data,"LED OFF") then
		nvm.set("led",false,"SMS")
		send(num,"LED关闭显示！")
		return true
	end
end

--[[
函数名：reset
功能  ：通过发送"RESET"短信控制设备重启
参数  ：num,号码
        data，短信内容
返回值：true，成功，否则失败
]]
local function reset(num,data)
	if data=="RESET" then		
		send(num,"重启成功！")
        nvm.set("abnormal",false)
		sys.timer_start(rtos.restart,10000)
		return true
	end
end

--[[
函数名：callmode
功能  ：通过发送短信控制通话模式
参数  ：num,号码
        data，短信内容
返回值：true，成功，否则失败
]]
local function callmode(num,data)
	if string.match(data,"SW TH") then		
		send(num,"双向通话模式设置成功！")
		nvm.set("callDmode",true,"SMS")
		return true
	elseif string.match(data,"SW JT") then
		send(num,"单向通话模式设置成功！")
		nvm.set("callDmode",false,"SMS")
		return true		
	end
end

--短信处理函数表
local tsmshandle =
{
	lbsdw,
	setadminum,
	deladminum,
	--workmod,
	query,
	led,
	reset,
	callmode,
}

--[[
函数名：handle
功能  ：解析收到的短信
参数  ：num,号码
        data，短信内容
        datetime，收到短信的时间
返回值：true，解析成功，否则失败
]]
local function handle(num,data,datetime)
	local k,v
	for k,v in pairs(tsmshandle) do
		if v(num,data,datetime) then
			return true
		end
	end	
end

--存放收到的短信
local tnewsms = {}

--[[
函数名：readsms
功能  ：读取短信
参数  ：无
返回值：无
]]
local function readsms()
	if #tnewsms ~= 0 then
		sms.read(tnewsms[1])
	end
end

--[[
函数名：newsms
功能  ：接收新短信，把短信位置插入tnewsms表中
参数  ：pos短信位置
返回值：无
]]
local function newsms(pos)
	table.insert(tnewsms,pos)
	if #tnewsms == 1 then
		readsms()
	end
end

--[[
函数名：readcnf
功能  ：读取短信确认函数
参数  ：result,num,data,pos,datetime,name,total,idx,isn
返回值：无
]]
local function readcnf(result,num,data,pos,datetime,name,total,idx,isn)
    local d1,d2 = string.find(num,"^([%+]*86)")
    if d1 and d2 then
        num = string.sub(num,d2+1,-1)
    end
    print("smsapp readcnf num",num,pos,datetime,total,idx)
    -- 删除新短信
    sms.delete(tnewsms[1])
    table.remove(tnewsms,1)
    -- 解析新短信内容
    
    --如果为长短信，发送长短信合并请求
    if total and total >1 then
        sys.dispatch("LONG_SMS_MERGE",num, data,datetime,name,total,idx,isn)  
        readsms()--读取下一条新短信
        return
    end
    
    --发送短信上报后台请求
    sys.dispatch("SMS_RPT_REQ",num, data,datetime)  
    
    --短信内容不为空，开会解析短信指令
    if data then
        data = string.upper(common.ucs2betogb2312(common.hexstobins(data)))
        handle(num,data,datetime)
    end
    
    --读取下一条新短信
    readsms()
end

--[[
函数名：longsmsmergecnf
功能  ：长短信确合并确认函数
参数  ：res,num,data,t,alpha
返回值：无
]]
local function longsmsmergecnf(res,num,data,t,alpha)
    print("smsapp longsmsmergecnf num",num,data,t)
    --发送短信上报后台请求
    sys.dispatch("SMS_RPT_REQ",num, data,t)  
    --短信内容不为空，开会解析短信指令
    if data then
        data = string.upper(common.ucs2betogb2312(common.hexstobins(data)))
        handle(num,data,datetime)
    end
end

local batlowsms

--[[
函数名：chgind
功能  ：DEV_CHG_IND消息处理函数
参数  ：evt,val
返回值：true
]]
local function chgind(evt,val)
	print("chgind",evt,val,nvm.get("adminum"))
	--[[if evt=="BAT_LOW" and val and nvm.get("adminum")~="" then
		if not send(nvm.get("adminum"),"设备电量低，请及时充电！") then
			print("wait batlowsms")
			batlowsms = true
		else
			if nvm.get("workmod")=="GPS" then
				nvm.set("workmod","PWRGPS","LOWPWR")
			end
		end
	end]]
	return true
end

local waitpoweroffcnt,waitpoweroff = 0

--[[
函数名：keylngpresind
功能  ：长按消息处理函数，如果主人的号码不为空，则先向主人号码发送基站定位信息，然后拨打主人号码
参数  ：无
返回值：true
]]
local function keylngpresind()
	if nvm.get("adminum")~="" then
		--[[if send(nvm.get("adminum"),encellinfo()) then
			waitpoweroffcnt = waitpoweroffcnt + 1
			waitpoweroff = true
		end
		if send(nvm.get("adminum"),"设备即将关机！") then
			waitpoweroffcnt = waitpoweroffcnt + 1
			waitpoweroff = true
		end]]
		send(nvm.get("adminum"),encellinfo())
		cc.dial(nvm.get("adminum"),3000)
	end
	return true
end

local function sendcnf()
	--[[print("sendcnf",waitpoweroff,waitpoweroffcnt)
	if waitpoweroff then
		waitpoweroffcnt = waitpoweroffcnt - 1
		if waitpoweroffcnt <= 0 then
			waitpoweroff = false
			print("poweroff")
			sys.timer_start(rtos.poweroff,3000)
		end
	end]]
end

local smsrdy,callrdy
--[[
函数名：smsready
功能  ：SMS_READY消息处理函数，将smsrdy置为true
参数  ：无
返回值：true
]]
local function smsready()
	print("smsready",batlowsms,chg.getcharger())
	smsrdy = true
	--[[if callrdy and batlowsms and not chg.getcharger() and nvm.get("adminum")~="" then
		batlowsms = false
		send(nvm.get("adminum"),"设备电量低，请及时充电！")
		if nvm.get("workmod")=="GPS" then
			nvm.set("workmod","PWRGPS","LOWPWR")
		end
	end]]
	return true
end

--[[
函数名：callready
功能  ：CALL_READY消息处理函数，将callrdy置为true
参数  ：无
返回值：true
]]
local function callready()
	print("callready",batlowsms,chg.getcharger())
	callrdy = true
	--[[if smsrdy and batlowsms and not chg.getcharger() and nvm.get("adminum")~="" then
		batlowsms = false
		send(nvm.get("adminum"),"设备电量低，请及时充电！")
		if nvm.get("workmod")=="GPS" then
			nvm.set("workmod","PWRGPS","LOWPWR")
		end
	end]]
	return true
end

--注册app消息对应的处理函数
local smsapp =
{
	SMS_NEW_MSG_IND = newsms,
	SMS_READ_CNF = readcnf,
	DEV_CHG_IND = chgind,
	SMS_SEND_CNF = sendcnf,
	MMI_KEYPAD_LONGPRESS_IND = keylngpresind,
	SMS_READY = smsready,
	CALL_READY = callready,
	LONG_SMS_MERGR_CNF = longsmsmergecnf,
}

sys.regapp(smsapp)
net.setcengqueryperiod(30000)
