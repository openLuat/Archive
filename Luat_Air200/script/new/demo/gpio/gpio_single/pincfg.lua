require"pins"
module(...,package.seeall)

--如下配置了开源模块中所有可用作GPIO的引脚，每个配置只是演示需要
--用户最终需根据自己的需求自行修改

--pin值定义如下：
--pio.P0_XX：表示GPIOXX，例如pio.P0_15，表示GPIO15
--pio.P1_XX：表示GPOXX，例如pio.P1_8，表示GPO8

--dir值定义如下（默认值为pio.OUTPUT）：
--pio.OUTPUT：表示输出，初始化是输出低电平
--pio.OUTPUT1：表示输出，初始化是输出高电平
--pio.INPUT：表示输入，需要轮询输入的电平状态
--pio.INT：表示中断，电平状态发生变化时会上报消息，进入本模块的intmsg函数

--valid值定义如下（默认值为1）：
--valid的值跟pins.lua中的set、get接口配合使用
--dir为输出时，配合pins.set接口使用，pins.set的第一个参数如果为true，则会输出valid值表示的电平，0表示低电平，1表示高电平
--dir为输入或中断时，配合get接口使用，如果引脚的电平和valid的值一致，get接口返回true；否则返回false
--dir为中断时，cb为中断引脚的回调函数，有中断产生时，如果配置了cb，会调用cb，如果产生中断的电平和valid的值相同，则cb(true)，否则cb(false)

--等价于PIN22 = {pin=pio.P1_8,dir=pio.OUTPUT,valid=1}
--第22个引脚：GPO8，配置为输出，初始化输出低电平；valid=1，调用pins.set(true,PIN22),则输出高电平，调用pins.set(false,PIN22),则输出低电平
PIN22 = {pin=pio.P1_8}

--第23个引脚：GPO6；配置为输出，初始化输出高电平；valid=0，调用pins.set(true,PIN23),则输出低电平，调用pins.set(false,PIN23),则输出高电平
PIN23 = {pin=pio.P1_6,dir=pio.OUTPUT1,valid=0}

--如下三个配置含义和PIN22相似
PIN25 = {pin=pio.P0_14}
PIN26 = {pin=pio.P0_3}
PIN27 = {pin=pio.P0_1}


local function pin5cb(v)
	print("pin5cb",v)
end
--第5个引脚：GPIO6；配置为中断；valid=1
--intcb表示中断管脚的中断处理函数，产生中断时，如果为高电平，则回调intcb(true)；如果为低电平，则回调intcb(false)
--调用get(PIN5)时，如果为高电平，则返回true；如果为低电平，则返回false
PIN5 = {name="PIN5",pin=pio.P0_6,dir=pio.INT,valid=1,intcb=pin5cb}

--和PIN22相似
--PIN6 = {pin=pio.P0_15}

--第20个引脚：GPIO13；配置为输入；valid=0
--调用get(PIN20)时，如果为高电平，则返回false；如果为低电平，则返回true
PIN20 = {pin=pio.P0_13,dir=pio.INPUT,valid=0}

--如下三个配置含义和PIN22相似
PIN21 = {pin=pio.P0_8}
PIN16 = {pin=pio.P0_24}
PIN17 = {pin=pio.P0_25}

pins.reg(PIN22,PIN23,PIN25,PIN26,PIN27,PIN5,--[[PIN6,]]PIN20,PIN21,PIN16,PIN17)
