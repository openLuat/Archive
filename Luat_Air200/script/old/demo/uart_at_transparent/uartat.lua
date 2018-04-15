module(...,package.seeall)

require"pm"
--注意，本功能模块只能用于简单的验证AT功能，AT手册里面的所有AT命令并不是完全都能支持
--如果需要完整的AT功能并且休眠唤醒控制，请使用Air200T模块+Air200T模块的软件 或者 Air200模块+Air200T模块的软件
--串口ID,1对应uart1
--如果要修改为uart2，把UART_ID赋值为2即可
local UART_ID = 1

--[[
函数名：print
功能  ：打印接口，此文件中的所有打印都会加上mcuart前缀
参数  ：无
返回值：无
]]
local function print(...)
	_G.print("uartat",...)
end

--[[
函数名：read
功能  ：读取串口接收到的数据
参数  ：无
返回值：无
]]
local function read()
	local data = ""
	--底层core中，串口收到数据时：
	--如果接收缓冲区为空，则会以中断方式通知Lua脚本收到了新数据；
	--如果接收缓冲器不为空，则不会通知Lua脚本
	--所以Lua脚本中收到中断读串口数据时，每次都要把接收缓冲区中的数据全部读出，这样才能保证底层core中的新数据中断上来，此read函数中的while语句中就保证了这一点
	while true do
		data = uart.read(UART_ID,"*l",0)
		if not data or string.len(data) == 0 then break end
		--打开下面的打印会耗时
		print("read",data)
		ril.sendtransparentdata(data)
	end
end

--[[
函数名：write
功能  ：通过串口发送数据
参数  ：
		s：要发送的数据
返回值：无
]]
function write(s)
	uart.write(UART_ID,s)	
end

--保持系统处于唤醒状态，不会休眠
pm.wake("uartat")
--注册串口的数据接收函数，串口收到数据后，会以中断方式，调用read接口读取数据
sys.reguart(UART_ID,read)
--配置并且打开串口
uart.setup(UART_ID,115200,8,uart.PAR_NONE,uart.STOP_1)


