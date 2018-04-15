--[[
模块名称：指示灯
模块功能：根据需求对设备的某些状态用不同灯的颜色，灯的闪烁频率做出提示
模块最后修改时间：2017.02.13
]]
module(...,package.seeall)

local appid

--[[
1-5，优先级由低到高：
	IDLE:三灯长灭	
	CHGING:充电中，红灯常亮
	CHGFULL:充满，绿灯常亮，其余灯灭
	LOWVOL:如果充电器不在位，并且电压低于3.6V，红灯慢闪
	LINKERR:开机一分钟后设备与后台没有建立连接：绿灯慢闪，如果60秒没有变化，就自动灭掉；
	LINKSUC:开机一分钟后设备与后台连接成功：绿灯长亮，如果连续60秒没有变化，就自动灭掉；
	GPSFIXFAIL: 联网成功后再打开GPS没有定位成功：蓝灯慢闪，当打开GPS时间超过120秒钟后仍没有定位成功，就自动灭掉；
	GPSFIXSUC:GPS定位成功：蓝灯长亮，如果60秒钟没有变化，就自动灭掉；
	
	
	SHORTKEY:连续短按5次，绿灯闪一次
	LONGKEY:长按，蓝灯闪一次
	FAC:工厂测试中，红绿蓝交替闪烁
1-3，状态：
	INACTIVE:未激活
	PEND:等待被激活
	ACTIVE:激活
--]]
--控制灯的优先级
local IDLE,CHGING,CHGFULL,WORKSTA,LINKERR,LINKSUC,GPSFIXFAIL,GPSFIXSUC,SHORTKEY,LONGKEY,GSMSTA2,GSMSTA1,GPSSTA2,GPSSTA1,LOWVOL,FAC,PWOFF,PRIORITYCNT = 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,17
local INACTIVE,PEND,ACTIVE = 1,2,3
local tcause = {}
local linksuc,pwoffcause
--按照红绿蓝排序，每个灯占用2个位置，每个位置可以设置是否生效，是否点亮，以及对应的时间(默认1秒钟)
local tledpin = 
{
	[pmd.KP_LEDR]=0,
	[pmd.KP_LEDG]=0,
	[pmd.KP_LEDB]=0,
}
local tled,ledcnt,ledpos,ledidx = {},3,2,1

--[[
函数名：isvalid
功能  ：是否有有效的灯
参数  ：无
返回值：无
]]
local function isvalid()
	local i
	for i=1,ledpos*ledcnt do
		if tled[i].valid then return true end
	end
end

--[[
函数名：init
功能  ：初始化，只有IDLE为激活状态，其他优先级状态为非激活，按照红绿蓝排序，每个灯占用2个位置，每个位置设置生效，不点亮，以及对应的时间(默认1秒钟)
参数  ：无
返回值：无
]]
local function init()
	local i
	--只有IDLE为激活状态，其他优先级状态为非激活
	tcause[IDLE] = ACTIVE
	for i=IDLE+1,PRIORITYCNT do
		tcause[i] = INACTIVE
	end
	
    --按照红绿蓝排序，每个灯占用2个位置，每个位置设置生效，不点亮，以及对应的时间(默认1秒钟)
	for i=1,ledpos*ledcnt do
		tled[i] = {}
		tled[i].pin = (i > ledpos*(ledcnt-1)) and pmd.KP_LEDB or((i > ledpos*(ledcnt-2)) and pmd.KP_LEDG or pmd.KP_LEDR)
		tled[i].valid = true
		tled[i].on = false
		tled[i].prd = 1000
	end
end

--[[
函数名：starttimer
功能  ：定时器
参数  ：idx,cb,prd
返回值：无
]]
local function starttimer(idx,cb,prd)
	if tcause[idx] == ACTIVE and not sys.timer_is_active(cb) then
		sys.timer_start(cb,prd)
	end
end

--[[
函数名：proc
功能  ：控制灯的亮灭
参数  ：无
返回值：无
]]
local function proc()
	if not isvalid() then return end
	local i = ledidx
	while true do
		if tled[i].valid then			
			print("light.proc",i,tled[i].on,tled[i].prd)
			local k,v
			for k,v in pairs(tledpin) do
				if k ~= tled[i].pin and v ~= 0 then
					pmd.ldoset(0,k)
					tledpin[k] = 0
					print("light.ldo",k,0)
				end
			end
			local flag,pin = (tled[i].on and 1 or 0),tled[i].pin
			if tledpin[pin] ~= flag then
				pmd.ldoset(flag,pin)
				tledpin[pin] = flag
				--print("light.ldo",pin,flag)
			end			
			starttimer(LONGKEY,longkeyend,500)
			starttimer(SHORTKEY,shortkeyend,500)
			sys.timer_start(proc,tled[i].prd)
			ledidx = (i+1 > ledcnt*ledpos) and 1 or (i+1)
			if tcause[IDLE] == ACTIVE then
				tled[i].valid = false
			end
			return
		else
			i = (i+1 > ledcnt*ledpos) and 1 or (i+1)
		end
	end
end

--[[
函数名：updflicker
功能  ：控制灯的闪烁
参数  ：head,tail,onprd,offprd
返回值：无
]]
local function updflicker(head,tail,onprd,offprd)
	--print("light.updflicker",head,tail,onprd,offprd)
	local j
	--[[for j=1,ledpos*ledcnt do
		print("light tled["..j.."].valid",tled[j].valid)
		print("light tled["..j.."].on",tled[j].on)
		print("light tled["..j.."].onprd",tled[j].prd)
	end]]
	for j=1,ledpos*ledcnt do
		if j>=head and j<=tail then
			tled[j].valid = true
			tled[j].on = (j%ledpos == 1)
			tled[j].prd = tled[j].on and onprd or offprd
		else
			tled[j].valid = false
		end			
	end
	--[[for j=1,ledpos*ledcnt do
		print("light tled["..j.."].valid",tled[j].valid)
		print("light tled["..j.."].on",tled[j].on)
		print("light tled["..j.."].prd",tled[j].onprd)
	end]]
end

--[[
函数名：updled
功能  ：加载灯的显示
参数  ：无
返回值：无
]]
local function updled()
	local i,idx
	for i=IDLE,PRIORITYCNT do
		--print("light.updled",i,tcause[i])
		if tcause[i] == ACTIVE then idx=i break end
	end
	print("light.updled",idx)
	if idx == FAC then
		updflicker(1,6,1000,1000)
	elseif idx == PWOFF then
		updflicker(1,6,1000,1000)
	elseif idx == LONGKEY then
		updflicker(5,5,500)
	elseif idx == SHORTKEY then
		updflicker(3,3,500)
	elseif idx == CHGFULL then
		updflicker(3,3,1000)	
	elseif idx == CHGING then
		updflicker(1,1,1000)
	elseif idx == GPSFIXFAIL then
		updflicker(5,6,1000,1000)
	elseif idx == GPSFIXSUC then
		updflicker(5,5,1000)
	elseif idx == WORKSTA then
		updflicker(3,4,1000,1000)
	elseif idx == LINKERR then
		updflicker(3,4,1000,1000)
	elseif idx == LINKSUC then
		updflicker(3,3,1000)
	elseif idx == LOWVOL then
		updflicker(1,2,1000,1000)
	elseif idx == GSMSTA1 then
		updflicker(3,4,100,200)
	elseif idx == GSMSTA2 then
		updflicker(3,3,1000)
	elseif idx == GPSSTA1 then
		updflicker(5,6,100,200)
	elseif idx == GPSSTA2 then
		updflicker(5,5,1000)
	elseif idx == IDLE then
		updflicker(2,2,1000,1000)
	end
	if not sys.timer_is_active(proc) then proc() end
end

--[[
函数名：updcause
功能  ：管理灯的显示
参数  ：idx优先级,val true打开灯 ，false关闭灯
返回值：无
]]
local function updcause(idx,val)
	local i,pend,upd
	print("light.updcause",idx,val)
	for i=1,PRIORITYCNT do
		print("light tcause["..i.."]="..tcause[i])
	end
	if val then--打开灯的显示
        --如果有优先级更高的状态灯正在显示或等待显示，加入等待队列
		for i=idx+1,PRIORITYCNT do
			if tcause[i] == PEND or tcause[i] == ACTIVE then
				tcause[idx] = PEND
				pend = true
				break
			end
		end
        --如果等待队列为空且本状态不为激活状态，将本状态设为激活状态
		if not pend and tcause[idx] ~= ACTIVE then
			tcause[idx] = ACTIVE
            --如果低于本优先级的状态有为激活状态的，则将低于本状态的状态灯设为等待状态
			for i=1,idx-1 do
				if tcause[i] == ACTIVE then tcause[i] = PEND end
			end
			upd = true
		end
	else--关闭灯的显示
		if tcause[idx] == ACTIVE then
			for i=idx-1,1,-1 do
				if tcause[i] == PEND then tcause[i] = ACTIVE break end
			end
			upd = true
		end
		tcause[idx] = INACTIVE
	end
	--[[print("light.updcause",pend,upd)
	for i=1,PRIORITYCNT do
		print("light tcause["..i.."]="..tcause[i])
	end]]
	if upd then updled() end
end

--[[
函数名：chgind
功能  ：插入充电器充电中：红灯常亮；
        插入充电器充满：绿灯长亮；
        充电器不在位：红灯（或者绿灯）灭。
        低电压提示方法： 如果充电器不在位，并且电压低于3.6V，红灯慢闪；
参数  ：evt,val
返回值：true
]]
local function chgind(evt,val)
	print("light.chgind",evt,val,pwoffcause)
	if evt == "CHG_STATUS" then
		if val == 0 then
			updcause(CHGING,false)
			updcause(CHGFULL,false)
		elseif val == 1 then
			if pwoffcause == "VOL_LEV0" or pwoffcause == "BAT_LOW" then
				updcause(PWOFF,false)
			end
			updcause(LOWVOL,false)
			updcause(CHGFULL,false)
			updcause(CHGING,true)
		elseif val == 2 then
			if pwoffcause == "VOL_LEV0" or pwoffcause == "BAT_LOW" then
				updcause(PWOFF,false)
			end
			updcause(LOWVOL,false)
			updcause(CHGING,false)
			updcause(CHGFULL,true)
		end
	elseif evt == "BAT_LOW1" then
		if val then
			updcause(LOWVOL,true)
		else
			updcause(LOWVOL,false)
		end
	end
	return true
end

local keycnt,longkeycnt,leftm,keyflag=0,0,32
--[[
函数名：resetkey
功能  ：恢复控制短按按键灯显示的一些变量值
参数  ：无
返回值：无
]]
local function resetkey()
	keycnt,leftm,keyflag = keycnt+1,32
	--updcause(GSMSTA2,false)
end

--[[
函数名：checkleftm
功能  ：计算长按或短按时灯显示的剩余时间，如果剩余时间不够3秒，显示3秒
参数  ：无
返回值：无
]]
local function checkleftm()
	print("light.checkleftm:",leftm)
	if leftm>=4 then
		sys.timer_start(checkleftm,1000)
		leftm = leftm-1
	end
end

--[[
函数名：keyind
功能  ：短按开机键：绿灯快闪2秒然后常亮，表示GSM连接正常，再按灯灭（不管短按长按），不按键则30秒后自动熄灭。
        绿灯一直快闪，表示GSM未正常连接，再按灯灭（不管短按长按），不按键则30秒后自动熄灭。
参数  ：无
返回值：无
]]
local function keyind()
	print("light.keyind ",keycnt,linksuc,keyflag)
	if keyflag == "LNGKEY" then
		keylngpresind()
		return true
	end
	keyflag = "SHORTKEY"
	keycnt = keycnt+1
	if keycnt%2 == 1 then--提示GSM连接状态
		updcause(GSMSTA1,true)       
		if linksuc then
			sys.timer_start(updcause,2000,GSMSTA1,false)
			updcause(GSMSTA2,true)
			sys.timer_start(updcause,32000,GSMSTA2,false)            
		else
			sys.timer_start(updcause,32000,GSMSTA1,false)
		end
		sys.timer_start(checkleftm,1000)
		sys.timer_start(resetkey,32000)
	else--关闭GSM连接状态提示
		sys.timer_stop(updcause,GSMSTA1,false)
		sys.timer_stop(updcause,GSMSTA2,false)
		sys.timer_stop(resetkey)
		sys.timer_stop(checkleftm)
		updcause(GSMSTA1,false)
		updcause(GSMSTA2,false)
		leftm,keyflag=32
	end
	return true
end

--[[
函数名：resetlongkey
功能  ：恢复控制长按按键灯显示的一些变量值
参数  ：无
返回值：无
]]
local function resetlongkey()
	longkeycnt,leftm,keyflag = longkeycnt+1,32
	--updcause(GPSSTA2,false)
end

--[[
函数名：keylngpresind
功能  ：长按开机键：蓝灯先快闪然后常亮不超过30秒，表示GPS定位正常，
        再按灯灭（不管短按长按），不按键则30秒后自动熄灭。
参数  ：无
返回值：true
]]
function keylngpresind()
	print("light.keylngpresind ",longkeycnt,gps.isfix())
	longkeycnt = longkeycnt+1
	keyflag = "LNGKEY"
	if longkeycnt%2 == 1 then--提示GPS连接状态
        --蓝灯快闪
		updcause(GPSSTA1,true) 
        --如果定位成功，蓝灯快闪变成蓝灯常亮30秒
        --否则定位失败，蓝灯快闪32秒后蓝灯灭掉
		if gps.isfix() then
			sys.timer_start(updcause,2000,GPSSTA1,false)
			updcause(GPSSTA2,true)
			sys.timer_start(updcause,32000,GPSSTA2,false)
		else
			sys.timer_start(updcause,32000,GPSSTA1,false)
		end
		sys.timer_start(checkleftm,1000)
		sys.timer_start(resetlongkey,32000)
	else--关闭GPS连接状态提示
		sys.timer_stop(updcause,GPSSTA1,false)
		sys.timer_stop(updcause,GPSSTA2,false)
		sys.timer_stop(resetlongkey)
		sys.timer_stop(checkleftm)
		updcause(GPSSTA1,false)
		updcause(GPSSTA2,false)
		leftm,keyflag=32
	end
	return true
end

--[[
函数名：shortkeyend
功能  ：关闭短按按键灯显示
参数  ：无
返回值：无
]]
function shortkeyend()
	updcause(SHORTKEY,false)
end

--[[
函数名：longkeyend
功能  ：关闭长按按键灯显示
参数  ：无
返回值：无
]]
function longkeyend()
	updcause(LONGKEY,false)
end

--[[
function chingend()
	updcause(CHGING,false)
end

local netok,neterr
local function netind(sta)
	if sta == "REGISTERED" then
		if not netok then
			updcause(WORKSTA,true)
			sys.timer_start(updcause,60000,WORKSTA,false)
			netok = true
		end
	else
		if not neterr then
			updcause(WORKSTA,false)
			sys.timer_stop(updcause,WORKSTA,false)
			neterr = true
		end
	end
	return true
end]]

--[[
函数名：linkerr
功能  ：连网失败且非异常重启，绿灯慢闪60秒
参数  ：无
返回值：无
]]
local function linkerr()
	if rtos.poweron_reason()==rtos.POWERON_RESTART and nvm.get("abnormal") then return end    
	if not linksuc then
		--updcause(WORKSTA,false)
		--sys.timer_stop(updcause,WORKSTA,false)
		updcause(LINKERR,true)
		sys.timer_start(updcause,600000,LINKERR,false)
	end
end

--[[
函数名：linkconsuc
功能  ：设备与后台连接成功
参数  ：无
返回值：true
]]
local function linkconsuc()
	print("light.linkconsuc ",keyflag)
	linksuc = true
    --如果有短按按键按下，则绿灯常亮leftm(<=30)秒
	if keyflag =="SHORTKEY" then
		sys.timer_stop(checkleftm)
		updcause(GSMSTA1,false)
		sys.timer_stop(updcause,GSMSTA1,false)  
		updcause(GSMSTA2,true)
		sys.timer_start(updcause,leftm*1000,GSMSTA2,false)
	end
    --如果是异常重启，开机连网成功不做灯提示
	if rtos.poweron_reason()==rtos.POWERON_RESTART and nvm.get("abnormal") then return true end	
	--开机连网成功绿灯常亮60秒
    sys.timer_stop(linkerr)
	updcause(LINKERR,false)
	sys.timer_stop(updcause,LINKERR,false)
	updcause(LINKSUC,true)
	sys.timer_start(updcause,60000,LINKSUC,false)
	return true
end

--[[
函数名：linkconfail
功能  ：设备与后台断开连接
参数  ：无
返回值：true
]]
local function linkconfail()
	print("light.linkconfail ")
	linksuc = false
	return true
end

local gpsfirstfix,gpsfstopn = true,true
--[[
函数名：gpsfixsuc
功能  ：gps首次定位成功且非异常重启蓝灯常亮60秒
        长按开机键：蓝灯先快闪2秒然后常亮不超过30秒，表示GPS定位正常，再按灯灭（不管短按长按），不按键则30秒后自动熄灭。
参数  ：无
返回值：true
]]
local function gpsfixsuc()
	print("light.gpsfixsuc ",keyflag,gpsfirstfix)
	if gpsfirstfix then
		gpsfirstfix = nil
		if rtos.poweron_reason()==rtos.POWERON_RESTART and nvm.get("abnormal") then
		else
			updcause(GPSFIXFAIL,false)
			sys.timer_stop(updcause,GPSFIXFAIL,false)
			updcause(GPSFIXSUC,true)
			sys.timer_start(updcause,60000,GPSFIXSUC,false)
		end	
	elseif keyflag=="LNGKEY" then
		sys.timer_stop(checkleftm)
		updcause(GPSSTA1,false)
		sys.timer_stop(updcause,GPSSTA1,false) 
		updcause(GPSSTA2,true)
		sys.timer_start(updcause,leftm*1000,GPSSTA2,false)
	end
	return true
end

--[[
函数名：gpsfstopnind
功能  ：首次打开gps，如果是异常重启，首次打开gps灯不做提示
参数  ：无
返回值：无
]]
local function gpsfstopnind()
	print("light.gpsfstopnind ")
	if gpsfstopn then
		gpsfstopn = nil
		if rtos.poweron_reason()==rtos.POWERON_RESTART and nvm.get("abnormal") then return true end
		updcause(GPSFIXFAIL,true)
		sys.timer_start(updcause,120000,GPSFIXFAIL,false)
	end
end

--[[
函数名：facind
功能  ：收到工程测试消息
参数  ：无
返回值：无
]]
local function facind()
	updcause(FAC,true)
end

--[[
函数名：rsp_pwoff
功能  ：关机请求应答
参数  ：cause关机原因值
返回值：无
]]
local function rsp_pwoff(cause)
	print("light rsp_pwoff",cause)
	pwoffcause=cause
	updcause(PWOFF,true)
end

local procer =
{
	DEV_CHG_IND = chgind,
	--NET_STATE_CHANGED = netind,
	MMI_KEYPAD_LONGPRESS_IND = keylngpresind,
	MMI_KEYPAD_IND = keyind,
	LINKAIR_CONNECT_SUC = linkconsuc,
	LINKAIR_CONNECT_FAIL = linkconfail,
	GPS_FIX_SUC = gpsfixsuc,
	GPS_FST_OPN = gpsfstopnind,
	FAC_IND = facind,
	REQ_PWOFF = rsp_pwoff,
}

--[[
函数名：updmodule
功能  ：加载灯显示
参数  ：无
返回值：无
]]
local function updmodule()
	init()
	if nvm.get("led") then
		appid = sys.regapp(procer)
		proc()
		chgind("CHG_STATUS",chg.getstate())
	else
		sys.timer_stop(longkeyend)
		sys.timer_stop(shortkeyend)
		sys.timer_stop(proc)
		sys.timer_stop(updcause,WORKSTA,false)
		sys.timer_stop(linkerr)
		sys.timer_stop(updcause,LINKERR,false)
		sys.timer_stop(updcause,GPSFIXSUC,false)
		pmd.ldoset(0,pmd.LDO_SINK)
		pmd.ldoset(0,pmd.LDO_KEYPAD)
		pins.set(false,pins.LIGHTB)
		if appid then
			sys.deregapp(appid)
			appid = nil
		end
	end
end

--[[
函数名：parachangeind
功能  ：如果控制led是否工作的变量发送变化，重新加载灯显示
参数  ：e,k修改的参数名
返回值：true
]]
local function parachangeind(e,k)
	if k == "led" then updmodule() end
	return true
end

--注册PARA_CHANGED_IND消息处理函数
sys.regapp(parachangeind,"PARA_CHANGED_IND")
--加载灯显示
updmodule()
--开机未连上后台，灯做出相应提示
linkerr()--sys.timer_start(linkerr,60000)

