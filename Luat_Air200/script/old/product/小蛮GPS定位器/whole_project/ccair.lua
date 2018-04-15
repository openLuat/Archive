--[[
模块名称：CC上层应用模块
模块功能：来电去电相关接口管理
模块最后修改时间：2017.02.13
]]

--定义模块,导入依赖库
module(...,package.seeall)
require"cc"

--stat用户自定义的CC状态信息:"IDLE","DIALING","CONNECT"
--num存储待呼出的电话号码
--linkshut网络断开标识，暂没用到
local stat,num,linkshut = "IDLE",{}
--starttm 来电或去电的开始时间
--totaltm 来电或去电的通话总时长
--ringtm 来电或去电的响铃时长
--cctyp 电话类型，0：来电，1：去电
--ccnum 来电或去电的号码
--tmtemp 用来计算电话总时长，响铃时长而设计的变量
local starttm,totaltm,ringtm,cctyp,ccnum,tmtemp--cctyp:0：来电，1：去电

--[[
函数名：checktm
功能  ：每进一次此函数，将tmtemp值加1
参数  ：
		无
返回值：无
]]
local function checktm()
    tmtemp=tmtemp+1
end

--[[
函数名：addnum
功能  ：向num表中加入带呼出的号码
参数  ：id,app消息id
		val 需要插入num表中的号码值
返回值：无
]]
local function addnum(id,val)
	print("ccapp addmun",id,val,stat)
    --只有号码不为空且当前状态为"IDLE"时才将号码添加到num表中
	if val and string.len(val) > 0 and stat == "IDLE" then
		table.insert(num,val)
	end
end

--[[
函数名：dialnum
功能  ：电话呼出
参数  ：无
返回值：呼出成功为true，否则为nil
]]
local function dialnum()
	print("ccapp dialnum",#num)
	if #num > 0 then		
		--link.shut()
		--linkshut = true
        --设置音频通道
		if nvm.get("callDmode") then
			audio.setaudiochannel(audiocore.LOUDSPEAKER)
		else
			audio.setaudiochannel(audiocore.AUX_HANDSET)	
		end	
        --取表中的第一个号码，并将表中的第一个号码删除
		ccnum = table.remove(num,1)	
        --开始呼出，如果呼出不成功，则继续拨打下一个号码
		if not cc.dial(ccnum,2000) then dialnum() end
        --标记电话类型cctyp为1即去电，将tmtemp清零，以便统计响铃时间
		cctyp,tmtemp=1,0 
        --获取去电的起始时间
		starttm = misc.getclockstr()
        --表现当前状态为去电状态
		stat = "DIALING"
        --如果一直没人接听，设置40秒自动挂断功能
		sys.timer_start(cc.hangup,40000,"r1")
        --开始计算响铃时间
		sys.timer_loop_start(checktm,1000)
		return true
	end
end

--[[
函数名：connect
功能  ：电话接通处理
参数  ：无
返回值：true
]]
local function connect()
    --如果电话接通，关掉40秒后自动挂断的定时器
	sys.timer_stop(cc.hangup,"r1")
    --标记当前状态为接通状态
	stat = "CONNECT"
    --存放号码的num表清空，记下响铃时间
	num,ringtm = {},tmtemp
    --tmtemp再次清0，开始计算通话时间
	tmtemp=0
	sys.dispatch("CCAPP_CONNECT")
	return true
end

--[[
函数名：disconnect
功能  ：电话挂断处理
参数  ：无
返回值：true
]]
local function disconnect()
    --关闭自动接听，自动挂断定时器
	sys.timer_stop(cc.accept)
	sys.timer_stop(cc.hangup,"r1")
	--[[if linkshut then
		linkshut = nil
		link.reset()
	end]]
    --关闭计算响铃时长/通话时长的定时器
	sys.timer_stop(checktm)	
    --计算响铃时长，通话总时长
	if stat ~= "CONNECT" then
	    ringtm=tmtemp
	    totaltm=0
	else
	    totaltm = tmtemp
	end
	tmtemp=0
    --发送电话上报请求
	sys.dispatch("CCRPT_REQ",cctyp,ccnum,starttm,ringtm,totaltm)
	print("ccair CCRPT_REQ",cctyp,ccnum,starttm,ringtm,totaltm)
	if not dialnum() then
		stat = "IDLE"
		sys.dispatch("CCAPP_DISCONNECT")
	end
	--sys.restart("restart with cc disconnect") 
	return true
end

--[[
函数名：incoming
功能  ：来电处理
参数  ：typ：app消息id
        num：来电号码
返回值：无
]]
local function incoming(typ,num)
    --如果工作模式是省电模式，则挂掉电话
	if nvm.get("workmod")=="PWRGPS" then
		cc.hangup()
		return
	end

    --如果adminum号不为空，且来电号码不是adminum号码，则挂掉电话
	if nvm.get("adminum")~="" then
		if num~="" and num~=nil and num~=nvm.get("adminum") then
			cc.hangup()
			return
		end
	end
	--link.shut()
	--linkshut = true
    --标记电话类型cctyp为0即来电，将tmtemp清0以便计算响铃时间，将来电号码存储在ccnum
	cctyp,tmtemp,ccnum=0,0,num
    --获取来电开始时间
	starttm = misc.getclockstr()
	print("ccair incoming",cctyp,ccnum,starttm,ringtm,totaltm)
    --设置音频通道
	if nvm.get("callDmode") then
		audio.setaudiochannel(audiocore.LOUDSPEAKER)
	else
		audio.setaudiochannel(audiocore.AUX_HANDSET)	
	end	
    --10S后自动接听
	sys.timer_start(cc.accept,10000)
    --开启计算响铃时间的定时器
	sys.timer_loop_start(checktm,1000)
end

--注册app处理函数
sys.regapp(incoming,"CALL_INCOMING")
sys.regapp(connect,"CALL_CONNECTED")
sys.regapp(disconnect,"CALL_DISCONNECTED")
sys.regapp(addnum,"CCAPP_ADD_NUM")
sys.regapp(dialnum,"CCAPP_DIAL_NUM")
--设置音频通道
audio.setaudiochannel(audiocore.AUX_HANDSET)
--设置MIC增益
audio.setmicrophonegain(7)
