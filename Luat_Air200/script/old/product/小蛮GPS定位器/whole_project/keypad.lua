--[[
模块名称：按键消息处理模块
模块功能：初始化按键矩阵，检测按键按下弹起消息，并分发按键消息
模块最后修改时间：2017.02.09
]]
module(...,package.seeall)

--curkey当前检测到的按键
local curkey
--长按时间值
local KEY_LONG_PRESS_TIME_PERIOD = 3000
KEY_SOS = "SOS"
--按键映射表
local keymap = {["12"] = KEY_SOS}
local sta = "IDLE"

--[[
函数名：keylongpresstimerfun
功能  ：长按按键定时器处理函数
参数  ：无
返回值：无
]]
local function keylongpresstimerfun ()
    --如果当前按键值不为nil，则分发长按按键消息
	if curkey then
		sys.dispatch("MMI_KEYPAD_LONGPRESS_IND",curkey)
		sta = "LONG"
	end
end

--[[
函数名：stopkeylongpress
功能  ：关闭长按按键处理函数的定时器
参数  ：无
返回值：无
]]
local function stopkeylongpress()
	curkey = nil
	sys.timer_stop(keylongpresstimerfun)
end

--[[
函数名：startkeylongpress
功能  ：开启长按按键处理函数的定时器
参数  ：key 当前检测到的按键
返回值：无
]]
local function startkeylongpress(key)
	stopkeylongpress()
	curkey = key
	sys.timer_start(keylongpresstimerfun,KEY_LONG_PRESS_TIME_PERIOD)
end

--[[
函数名：keymsg
功能  ：处理按键的按下，弹起状态，检测当前按下按键值
参数  ：msg 按键消息
返回值：无
]]
local function keymsg(msg)
	print("keypad.keymsg",msg.key_matrix_row,msg.key_matrix_col)
	local key = keymap[msg.key_matrix_row..msg.key_matrix_col]
	if key then
		if msg.pressed then
            --检测到按键按下，标记状态我按下，开启长按定时处理定时器
			sta = "PRESSED"
			startkeylongpress(key)			
		else
            --检测到按键弹起，1关掉长按定时处理定时器,2如果按键状态为按下，则分发短按消息，3恢复按键状态
			stopkeylongpress()
			if sta == "PRESSED" then
				sys.dispatch("MMI_KEYPAD_IND",key)
			end
			sta = "IDLE"
		end
	end
end

--计算短按消息的次数
local fivetap = 0

--[[
函数名：resetfivetap
功能  ：将计算短按次数的fivetap恢复为0
参数  ：无
返回值：无
]]
local function resetfivetap()
	fivetap = 0
end

--[[
函数名：keyind
功能  ：处理短按消息，如果1秒内有大于等于5次短按发生，则分发MMI_KEYPAD_FIVETAP_IND消息
参数  ：无
返回值：true
]]
local function keyind()
	fivetap = fivetap+1
	if fivetap >= 5 then
		resetfivetap()
		sys.timer_stop(resetfivetap)
		sys.dispatch("MMI_KEYPAD_FIVETAP_IND")
	else
		sys.timer_start(resetfivetap,1000)
	end
	return true
end

--注册按键消息处理函数
sys.regmsg(rtos.MSG_KEYPAD,keymsg)
--注册短按消息处理函数
sys.regapp(keyind,"MMI_KEYPAD_IND")
--初始化按键矩阵
rtos.init_module(rtos.MOD_KEYPAD,0,0x04,0x02)
