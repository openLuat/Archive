require"pincfg"
module(...,package.seeall)

--[[
函数名：print
功能  ：打印接口，此文件中的所有打印都会加上test前缀
参数  ：无
返回值：无
]]
local function print(...)
	_G.print("test",...)
end

-------------------------PIN31测试开始-------------------------
local pin31flg = true
--[[
函数名：pin31set
功能  ：设置PIN31引脚的输出电平，1秒反转一次
参数  ：无
返回值：无
]]
local function pin31set()
	pins.set(pin31flg,pincfg.PIN31)
	pin31flg = not pin31flg
end
--启动1秒的循环定时器，设置PIN31引脚的输出电平
sys.timer_loop_start(pin31set,1000)
-------------------------PIN31测试结束-------------------------


-------------------------PIN32测试开始-------------------------
local pin32flg = true
--[[
函数名：pin32set
功能  ：设置PIN32引脚的输出电平，1秒反转一次
参数  ：无
返回值：无
]]
local function pin32set()
	pins.set(pin32flg,pincfg.PIN32)
	pin32flg = not pin32flg
end
--启动1秒的循环定时器，设置PIN32引脚的输出电平
sys.timer_loop_start(pin32set,1000)
-------------------------PIN32测试结束-------------------------


-------------------------PIN28测试开始-------------------------
--[[
函数名：pin28get
功能  ：读取PIN28引脚的输入电平
参数  ：无
返回值：无
]]
local function pin28get()
	local v = pins.get(pincfg.PIN28)
	print("pin28get",v and "low" or "high")
end
--启动1秒的循环定时器，读取PIN28引脚的输入电平
sys.timer_loop_start(pin28get,1000)
-------------------------PIN28测试结束-------------------------
