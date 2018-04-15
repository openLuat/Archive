--[[
模块名称：看门狗模块
模块功能：MCU与153B单片机实现双向喂狗功能，防止MCU死机
模块最后修改时间：2017.02.13
]]
module(...,package.seeall)

local typ = 1 --0:SGM706; 1:EM78P153B
local scm_active,get_scm_cnt = true,20
local testcnt,testing = 0

--[[
local function change()
	watchdog.kick()
	sys.timer_start(change,1000)
end]]

--[[
函数名：getscm
功能  ：判断单片机有没有定时给MCU喂狗
参数  ：tag，喂狗标签，标记是正常喂狗还是看门狗测试时喂狗
返回值：无
]]
local function getscm(tag)
	if tag=="normal" and testing then return end
	get_scm_cnt = get_scm_cnt - 1
	if tag=="test" then
		sys.timer_stop(getscm,"normal")
	end
	if get_scm_cnt > 0 then
		if tag=="test" then
			if not pins.get(pins.WATCHDOG) then				
				testcnt = testcnt+1
				if testcnt<3 then
					sys.timer_start(feed,100,"test")
					get_scm_cnt = 20
					return
				else
					testing = nil
				end
			end
		end
		sys.timer_start(getscm,100,tag)
	else
		get_scm_cnt = 20
		if tag=="test" then
			testing = nil
		end
	end

    --判断单片机是否有给MCU喂狗，如果有喂狗，scm_active喂true
	if pins.get(pins.WATCHDOG) then
		scm_active = true
		print("wdt scm_active = true")
	end
end

--[[
函数名：feedend
功能  ：结束向单片机喂狗，修改喂狗引脚为输入，以便读取喂狗信号判断单片机是否有向MCU定时喂狗
参数  ：tag，喂狗标签，标记是正常喂狗还是看门狗测试时喂狗
返回值：无
]]
local function feedend(tag)
	if tag=="normal" and testing then return end
	pins.setdir(pio.INPUT,pins.WATCHDOG)
	print("wdt feedend",tag)
	if tag=="test" then
		sys.timer_stop(getscm,"normal")
	end
	sys.timer_start(getscm,100,tag)
end

--[[
函数名：feed
功能  ：喂狗
参数  ：tag，喂狗标签，标记是正常喂狗还是看门狗测试时喂狗
返回值：无
]]
function feed(tag)
	if tag=="normal" and testing then return end
    --如果单片机有定时向MCU喂狗，将scm_active置false，如果单片机没定时向MCU喂狗，重启单片机
	if scm_active or tag=="test" then
		scm_active = false
	else
		pins.set(false,pins.RST_SCMWD)
		sys.timer_start(pins.set,100,true,pins.RST_SCMWD)
		print("wdt reset 153b",tag)
	end

    --给153B喂狗
	pins.setdir(pio.OUTPUT,pins.WATCHDOG)
	pins.set(true,pins.WATCHDOG)
	print("wdt feed",tag)

    --开启2分钟后再次向153B单片机喂狗的定时器
	sys.timer_start(feed,120000,"normal")
	if tag=="test" then
		sys.timer_stop(feedend,"normal")
	end
    --2秒后结束向单片机喂狗操作
	sys.timer_start(feedend,2000,tag)
end

--[[
函数名：open
功能  ：打开看门狗
参数  ：无
返回值：无
]]
local function open()
	if typ == 0 then
		--sys.timer_start(change,1000)
		--watchdog.open(watchdog.DEFAULT,pins.WATCHDOG.pin)
	elseif typ == 1 then
		sys.timer_start(feed,120000,"normal")
		--pins.set(false,pins.WATCHDOG)
	end
end

--[[
函数名：test
功能  ：测试接口函数
参数  ：无
返回值：无
]]
function test()
	if not testing then
		testcnt,testing = 0,true
		feed("test")
	end
end

--给项目名追加尾缀
_G.appendprj("_WD"..typ)
--定时开启看门狗
sys.timer_start(open,200)
--sys.timer_start(test,10000)
