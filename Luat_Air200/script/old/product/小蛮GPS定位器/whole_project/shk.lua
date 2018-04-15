--[[
模块名称：震动模块
模块功能：初始化gsensor，检测震动并分发震动消息
模块最后修改时间：2017.02.13
]]

module(...,package.seeall)

local i2cid,intregaddr = 1,0x1A
local initstr,indcnt1,indcnt2 = "",0,0

--[[
函数名：clrint
功能  ：清除中断
参数  ：无
返回值：无
]]
local function clrint()
	print("shk.clrint 1")
	if pins.get(pins.GSENSOR) then
		print("shk.clrint 2")
		i2c.read(i2cid,intregaddr,1)
	end
end

--[[
函数名：init2
功能  ：初始化gsensor2阶段
参数  ：无
返回值：无
]]
local function init2()
	local cmd,i = {0x1B,0x00,0x6A,0x01,0x1E,0x20,0x21,0x04,0x1B,0x00,0x1B,0xDA,0x1B,0xDA}
	--local cmd,i = {0x1B,0x00,0x6A,0x01,0x1E,0x20,0x21,0x04,0x1D,0x06,0x1B,0x1A,0x1B,0x9A}
	--local cmd,i = {0x1B,0x00,0x6A,0x01,0x1E,0x20,0x21,0x04,0x1B,0x1A,0x1B,0x9A}
	for i=1,#cmd,2 do
		i2c.write(i2cid,cmd[i],cmd[i+1])
		print("shk.init2",string.format("%02X",cmd[i]),string.format("%02X",string.byte(i2c.read(i2cid,cmd[i],1))))
		initstr = initstr..","..(string.format("%02X",cmd[i]) or "nil")..":"..(string.format("%02X",string.byte(i2c.read(i2cid,cmd[i],1))) or "nil")
	end
	clrint()
end

--[[
函数名：checkready
功能  ：检查gsensor是否准备就绪
参数  ：无
返回值：无
]]
local function checkready()
	local s = i2c.read(i2cid,0x1D,1)
	print("shk.checkready",s,(s and s~="") and string.byte(s) or "nil")
	if s and s~="" then
		if bit.band(string.byte(s),0x80)==0 then
			init2()
			return
		end
	end
	sys.timer_start(checkready,1000)
end

--[[
函数名：init
功能  ：初始化gsensor 1阶段
参数  ：无
返回值：无
]]
local function init()
	local i2cslaveaddr = 0x0E
	if i2c.setup(i2cid,i2c.SLOW,i2cslaveaddr) ~= i2c.SLOW then
		print("shk.init fail")
		initstr = "fail"
		return
	end
	i2c.write(i2cid,0x1D,0x80)
	sys.timer_start(checkready,1000)
end

--[[
函数名：getdebugstr
功能  ：获取调试信息（包含中断1中断2的上报次数）
参数  ：无
返回值：初始化调试信息+中断1中断2的上报次数
]]
function getdebugstr()
	return initstr..";".."indcnt1:"..indcnt1..";".."indcnt2:"..indcnt2
end

--[[
函数名：ind
功能  ：gsensor中断消息处理，分发震动消息
参数  ：无
返回值：无
]]
local function ind(id,data)
	print("shk.ind",id,data)
	if data then
		indcnt1 = indcnt1 + 1
	else
		indcnt2 = indcnt2 + 1
	end	
	if id == string.format("PIN_%s_IND",pins.GSENSOR.name) then
		if data then
			clrint()
			print("shk.ind DEV_SHK_IND")
			sys.dispatch("DEV_SHK_IND")
		end
	end
end

--注册中断处理函数
sys.regapp(ind,string.format("PIN_%s_IND",pins.GSENSOR.name))
--初始化gsensor
init()
--每30秒清一次中断
sys.timer_loop_start(clrint,30000)
