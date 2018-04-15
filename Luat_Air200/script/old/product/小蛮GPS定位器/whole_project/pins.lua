--[[
模块名称：引脚配置模块
模块功能：初始化各引脚
模块最后修改时间：2017.02.13
]]

module(...,package.seeall)

--gsensor引脚
GSENSOR = {name="GSENSOR",pin=pio.P0_3,dir=pio.INT,valid=0}
--喂狗信号引脚
WATCHDOG = {pin=pio.P0_14,init=false,valid=0}
--复位单片机引脚
RST_SCMWD = {pin=pio.P0_12,defval=true,valid=1}
--蓝灯控制引脚
LIGHTB = {pin=pio.P0_25}

--需要初始化的所有引脚的存放表
local allpin = {GSENSOR,WATCHDOG,RST_SCMWD}

--[[
函数名：get
功能  ：获取引脚的高低电平
参数  ：p
返回值：true valid对应的电平值，false valid对应的反电平值
]]
function get(p)
	if p.get then return p.get(p) end
	return pio.pin.getval(p.pin) == p.valid
end

--[[
函数名：set
功能  ：设置引脚的高低电平
参数  ：bval
        p
返回值：无
]]
function set(bval,p)
	p.val = bval

	if not p.inited and (not p.ptype or p.ptype == "GPIO") then
		p.inited = true
		pio.pin.setdir(p.dir or pio.OUTPUT,p.pin)
	end

	if p.set then p.set(bval,p) return end

	if p.ptype and p.ptype ~= "GPIO" then print("unknwon pin type:",p.ptype) return end

	local valid = p.valid == 0 and 0 or 1 -- 默认高有效
	local notvalid = p.valid == 0 and 1 or 0
	local val = bval == true and valid or notvalid

	if p.pin then pio.pin.setval(val,p.pin) end
end

--[[
函数名：setdir
功能  ：设置引脚的输入或输出
参数  ：dir
        p
返回值：
]]
function setdir(dir,p)
	if p and not p.ptype or p.ptype == "GPIO" then
		if not p.inited then
			p.inited = true
		end
		if p.pin then
			pio.pin.close(p.pin)
			pio.pin.setdir(dir,p.pin)
			p.dir = dir
		end
	end
end

--[[
函数名：init
功能  ：初始化各引脚
参数  ：无
返回值：无
]]
function init()
	for _,v in ipairs(allpin) do
		if v.init == false then
			-- 不做初始化
		elseif not v.ptype or v.ptype == "GPIO" then
			v.inited = true
			pio.pin.setdir(v.dir or pio.OUTPUT,v.pin)
			if not v.dir or v.dir == pio.OUTPUT then
				set(v.defval or false,v)
			elseif v.dir == pio.INPUT or v.dir == pio.INT then
				v.val = pio.pin.getval(v.pin) == v.valid
			end
		elseif v.set then
			set(v.defval or false,v)
		end
	end
end

--[[
函数名：intmsg
功能  ：中断消息处理函数，分发中断消息
参数  ：msg中断消息
返回值：无
]]
local function intmsg(msg)
	local status = 0

	if msg.int_id == cpu.INT_GPIO_POSEDGE then status = 1 end

	for _,v in ipairs(allpin) do
		if v.dir == pio.INT and msg.int_resnum == v.pin then
			v.val = v.valid == status
			sys.dispatch(string.format("PIN_%s_IND",v.name),v.val)
			return
		end
	end
end
--注册中断消息处理函数
sys.regmsg(rtos.MSG_INT,intmsg)
--初始化各引脚
init()
