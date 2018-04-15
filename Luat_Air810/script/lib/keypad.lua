module(...,package.seeall)

--[[
  DEV_DATACARD: 物联网模块
  DEV_TRACKER: 定位器模块
]]
DEV_DATACARD,DEV_TRACKER = 0,1

local KEY_LONG_PRESS_TIME_PERIOD = 1500
local keymap,keymode = {["255255"] = "K_RED"},DEV_DATACARD
local sta,curkey = "IDLE"
local fivetap,KEY_TAP_MAX = 0,5

local function print(...)
  _G.print("keypad",...)
end

function init_keypad(mode)
  keymode = mode
end

local function pwoff()
  print("pwoff")
  sys.poweroff()
end

local function repwron()
  print("repwron")
  sys.setPwrFlag(true)
  sys.repwron()
  sys.dispatch("PWRKEY_IND",sys.getPwrFlag())
end

local function keymodeprocess(mode)
  if mode == DEV_TRACKER then
    sys.dispatch("MMI_KEYPAD_LONGPRESS_IND",curkey)
  elseif mode == DEV_DATACARD then
    if sys.isPwronCharger() then
      if sys.getPwrFlag() then
        sys.timer_start(pwoff,100)
      else
        sys.timer_start(repwron,100)
      end
    else
      sys.timer_start(pwoff,100)
    end
  else
    print("keymode is invalid!",mode)
  end
end

local function keylongpresstimerfun()
  print("keylongpresstimerfun curkey",curkey,sys.isPwronCharger(),sys.getPwrFlag())
  if curkey == "K_RED" then
    keymodeprocess(keymode)
    sta = "LONG"
  end
end

local function stopkeylongpress()
	curkey = nil
	sys.timer_stop(keylongpresstimerfun)
end

local function startkeylongpress(key)
	print("startkeylongpress",curkey,key,sys.isPwronCharger(),sys.getPwrFlag())
	stopkeylongpress()
	curkey = key
	
	sys.timer_start(keylongpresstimerfun,KEY_LONG_PRESS_TIME_PERIOD)
end

local function keymsg(msg)
	print("keypad.keymsg",msg.key_matrix_row,msg.key_matrix_col)
	local key = keymap[msg.key_matrix_row..msg.key_matrix_col]
	print("keymsg key",key,msg.pressed)
	if key then
		if msg.pressed then
			sta = "PRESSED"
			startkeylongpress(key)
		else
			stopkeylongpress()
			if sta == "PRESSED" then
				sys.dispatch("MMI_KEYPAD_IND",key)
			end
			sta = "IDLE"
		end
	end
end

local function resetfivetap()
	fivetap = 0
end

local function keyind()
	fivetap = fivetap+1
	if fivetap >= KEY_TAP_MAX then
		resetfivetap()
		sys.timer_stop(resetfivetap)
		sys.dispatch("MMI_KEYPAD_FIVETAP_IND")
	else
		sys.timer_start(resetfivetap,1000)
	end
	return true
end

sys.regmsg(rtos.MSG_KEYPAD,keymsg)
sys.regapp(keyind,"MMI_KEYPAD_IND")
rtos.init_module(rtos.MOD_KEYPAD,0,0,0)
