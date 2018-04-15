module(...,package.seeall)

local appid

--[[
1-5，优先级由低到高：
	IDLE:三灯长灭	
	CHGING:充电中，红灯常亮
	CHGFULL:充满，绿灯常亮，其余灯灭
	LOWVOL:如果充电器不在位，并且电压低于3.6V，红灯慢闪
	LINKERR:开机一分钟后设备与后台没有建立连接：绿灯慢闪，如果600秒没有变化，就自动灭掉；
    LINKSUC:开机一分钟后设备与后台连接成功：绿灯长亮，如果连续600秒没有变化，就自动灭掉；
    GPSFIXFAIL: 联网成功后再打开GPS没有定位成功：蓝灯慢闪，当打开GPS时间超过120秒钟后仍没有定位成功，就自动灭掉；
    GPSFIXSUC:GPS定位成功：蓝灯长亮，如果120秒钟没有变化，就自动灭掉；
	
	SHORTKEY:连续短按5次，绿灯闪一次
	LONGKEY:长按，蓝灯闪一次
1-3，状态：
	INACTIVE:未激活
	PEND:等待被激活
	ACTIVE:激活
--]]
local IDLE,CHGING,CHGFULL,WORKSTA,LINKERR,LINKSUC,GPSFIXFAIL,GPSFIXSUC,SHORTKEY,LONGKEY,GSMSTA2,GSMSTA1,GPSSTA2,GPSSTA1,LOWVOL,PWOFF,PRIORITYCNT = 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,16
local INACTIVE,PEND,ACTIVE = 1,2,3
local tcause = {}
local linksuc,pwoffcause
--按照红绿蓝排序，每个灯占用2个位置，每个位置可以设置是否生效，是否点亮，以及对应的时间(默认1秒钟)
local tledpin = 
{
	[pmd.LDO_SINK]=0,
	[pmd.LDO_KEYPAD]=0,
	[pmd.KP_LEDB]=0,
}
local tled,ledcnt,ledpos,ledidx = {},3,2,1

local function print(...)
  _G.print("light",...)
end

local function isvalid()
	local i
	for i=1,ledpos*ledcnt do
		if tled[i].valid then return true end
	end
end

local function init()
	local i
	
	tcause[IDLE] = ACTIVE
	for i=IDLE+1,PRIORITYCNT do
		tcause[i] = INACTIVE
	end
	
	for i=1,ledpos*ledcnt do
		tled[i] = {}
		tled[i].pin = (i > ledpos*(ledcnt-1)) and pmd.KP_LEDB or((i > ledpos*(ledcnt-2)) and pmd.LDO_KEYPAD or pmd.LDO_SINK)
		tled[i].valid = true
		tled[i].on = false
		tled[i].prd = 1000
	end
	
    pins.setdir(pio.OUTPUT,pinscfg.LIGHTB)
    pins.set(false,pinscfg.LIGHTB)
end

local function starttimer(idx,cb,prd)
	if tcause[idx] == ACTIVE and not sys.timer_is_active(cb) then
		sys.timer_start(cb,prd)
	end
end

local function proc()
	if not isvalid() then return end
	local i = ledidx
	while true do
		if tled[i].valid then			
			print("proc",i,tled[i].on,tled[i].prd)
			local k,v
			for k,v in pairs(tledpin) do
				if k ~= tled[i].pin and v ~= 0 then
					if k ~= pmd.KP_LEDB then  
						pmd.ldoset(0,k)
					else
						pins.set(false,pinscfg.LIGHTB)
					end
					tledpin[k] = 0
					print("ldo",k,0)
				end
			end
			local flag,pin = (tled[i].on and 1 or 0),tled[i].pin
			if tledpin[pin] ~= flag then
				if pin ~= pmd.KP_LEDB then
					pmd.ldoset(flag,pin)
				else		
					pins.set(tled[i].on,pinscfg.LIGHTB)		
				end
				tledpin[pin] = flag
				--print("ldo",pin,flag)
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

local function updflicker(head,tail,onprd,offprd)
	--print("updflicker",head,tail,onprd,offprd)
	local j
	--[[for j=1,ledpos*ledcnt do
		print("tled["..j.."].valid",tled[j].valid)
		print("tled["..j.."].on",tled[j].on)
		print("tled["..j.."].onprd",tled[j].prd)
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
		print("tled["..j.."].valid",tled[j].valid)
		print("tled["..j.."].on",tled[j].on)
		print("tled["..j.."].prd",tled[j].onprd)
	end]]
end

local function updled()
	local i,idx
	for i=IDLE,PRIORITYCNT do
		--print("updled",i,tcause[i])
		if tcause[i] == ACTIVE then idx=i break end
	end
	print("updled",idx)
  if idx == PWOFF then
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

local function updcause(idx,val)
	local i,pend,upd
	print("light.updcause",idx,val)
	for i=1,PRIORITYCNT do
		print("light tcause["..i.."]="..tcause[i])
	end
	if val then
		for i=idx+1,PRIORITYCNT do
			if tcause[i] == PEND or tcause[i] == ACTIVE then
				tcause[idx] = PEND
				pend = true
				break
			end
		end
		if not pend and tcause[idx] ~= ACTIVE then
			tcause[idx] = ACTIVE
			for i=1,idx-1 do
				if tcause[i] == ACTIVE then tcause[i] = PEND end
			end
			upd = true
		end
	else
		if tcause[idx] == ACTIVE then
			for i=idx-1,1,-1 do
				if tcause[i] == PEND then tcause[i] = ACTIVE break end
			end
			upd = true
		end
		tcause[idx] = INACTIVE
	end
	--[[print("updcause",pend,upd)
	for i=1,PRIORITYCNT do
		print("tcause["..i.."]="..tcause[i])
	end]]
	if upd then updled() end
end

local function chgind(evt,val)
	print("chgind",evt,val,pwoffcause)
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
local function resetkey()
    keycnt,leftm,keyflag = keycnt+1,32
    --updcause(GSMSTA2,false)
end

local function checkleftm()
    print("light.checkleftm:",leftm)
    if leftm>=4 then
        sys.timer_start(checkleftm,1000)
        leftm = leftm-1
    end
end

local function keyind()
    print("keyind ",keycnt,linksuc,keyflag)
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

local function resetlongkey()
    longkeycnt,leftm,keyflag = longkeycnt+1,32
    --updcause(GPSSTA2,false)
end

function keylngpresind()
    print("keylngpresind ",longkeycnt,gps.isfix())
    longkeycnt = longkeycnt+1
    keyflag = "LNGKEY"
	if longkeycnt%2 == 1 then--提示GSM连接状态
        updcause(GPSSTA1,true)
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

function shortkeyend()
	updcause(SHORTKEY,false)
end

function longkeyend()
	updcause(LONGKEY,false)
end

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
end


local function linkerr()
    if rtos.poweron_reason()==rtos.POWERON_RESTART then return end    
	if not linksuc then
		--updcause(WORKSTA,false)
		--sys.timer_stop(updcause,WORKSTA,false)
		updcause(LINKERR,true)
		sys.timer_start(updcause,600000,LINKERR,false)
	end
end
local function linkconsuc()
    print("linkconsuc ",keyflag)
    linksuc = true
    if keyflag =="SHORTKEY" then
        sys.timer_stop(checkleftm)
        updcause(GSMSTA1,false)
        sys.timer_stop(updcause,GSMSTA1,false)  
        updcause(GSMSTA2,true)
        sys.timer_start(updcause,leftm*1000,GSMSTA2,false)
    end
    if rtos.poweron_reason()==rtos.POWERON_RESTART then return true end	
	sys.timer_stop(linkerr)
	updcause(LINKERR,false)
	sys.timer_stop(updcause,LINKERR,false)
	updcause(LINKSUC,true)
    sys.timer_start(updcause,60000,LINKSUC,false)
	return true
end

local function linkconfail()
    print("linkconfail ")
    linksuc = false
    return true
end

local gpsfirstfix,gpsfstopn = true,true
local function gpsfixsuc()
  print("gpsfixsuc ",keyflag,gpsfirstfix)
  if gpsfirstfix then
    gpsfirstfix = nil
    if rtos.poweron_reason()==rtos.POWERON_RESTART then
      --TO DO ...
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

local function gpsfstopnind()
    print("gpsfstopnind ")
    if gpsfstopn then
        gpsfstopn = nil
        if rtos.poweron_reason()==rtos.POWERON_RESTART then return true end
        updcause(GPSFIXFAIL,true)
        sys.timer_start(updcause,120000,GPSFIXFAIL,false)
    end
end

local function rsp_pwoff(cause)
    print("rsp_pwoff",cause)
    pwoffcause=cause
    updcause(PWOFF,true)
end

local procer =
{
	DEV_CHG_IND = chgind,
	MMI_KEYPAD_LONGPRESS_IND = keylngpresind,
	MMI_KEYPAD_IND = keyind,
	SOCKET_CONNECT_SUC = linkconsuc,
	SOCKET_CONNECT_FAIL = linkconfail,
	GPS_FIX_SUC = gpsfixsuc,
	GPS_FST_OPN = gpsfstopnind,
	REQ_PWOFF = rsp_pwoff,
}

local function updmodule()
	init()

	appid = sys.regapp(procer)
	proc()
	chgind("CHG_STATUS",chg.getstate())
end

local function parachangeind(e,k)
	if k == "led" then updmodule() end
	return true
end

sys.regapp(parachangeind,"PARA_CHANGED_IND")
updmodule()
linkerr()

